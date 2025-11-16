from typing import Optional, Iterable
import io
import os

try:
    import cairosvg
except Exception as e:
    raise ImportError("cairosvg is required. Install with: pip install cairosvg") from e

from PIL import Image


def svg_sequence_to_gif(
        n: int,
        output_path: str = "animation.gif",
        input_pattern: str = "{i}.svg",
        start_index: int = 0,
        duration_ms: int = 100,
        loop: int = 0,
        scale: float = 1.0,
        ping_pong: bool = False,
        bgcolor=(255, 255, 255, 255),
        verbose: bool = True,
) -> None:
    """
    Read a sequence of SVGs and write an animated GIF suitable for GitHub.

    Parameters
    ----------
    n : int
        Number of frames to read. Frames are read for indices start_index .. start_index + n - 1.
    output_path : str
        Path to write animated GIF.
    input_pattern : str
        Pattern for SVG filenames, must contain '{i}', e.g. '{i}.svg' or 'frames/frame_{i:03d}.svg'
    start_index : int
        Index of the first frame (default 0).
    duration_ms : int
        Frame duration in milliseconds.
    loop : int
        Number of times to loop the GIF. 0 means infinite loop.
    scale : float
        Scale factor applied during rasterization (1.0 keeps original SVG DPI).
    ping_pong : bool
        If True, append frames reversed (except the last frame) to create a back-and-forth animation.
    bgcolor : tuple
        RGBA background to composite onto (use opaque white by default) to avoid GIF artifacts.
    verbose : bool
        Print progress messages.
    """
    if n <= 0:
        raise ValueError("n must be > 0")

    frames: list[Image.Image] = []
    widths = []
    heights = []

    # Load each SVG -> PNG (in memory) -> PIL Image
    for idx in range(start_index, start_index + n):
        filename = input_pattern.format(i=idx)
        if verbose:
            print(f"Reading {filename} ...", end=" ")
        if not os.path.exists(filename):
            raise FileNotFoundError(f"SVG file not found: {filename}")

        with open(filename, "rb") as f:
            svg_bytes = f.read()

        # Convert SVG bytes to PNG bytes with cairosvg (in-memory)
        png_bytes = cairosvg.svg2png(bytestring=svg_bytes, scale=scale)
        img = Image.open(io.BytesIO(png_bytes)).convert("RGBA")
        widths.append(img.width)
        heights.append(img.height)
        frames.append(img)
        if verbose:
            print(f"ok (size={img.width}x{img.height})")

    # Determine target canvas size (use max so we don't crop)
    target_w = max(widths)
    target_h = max(heights)
    if verbose:
        print(f"Target canvas size: {target_w}x{target_h}")

    # Composite frames onto uniform background (so GIF frames match)
    composed_frames: list[Image.Image] = []
    for i, f in enumerate(frames):
        if f.size != (target_w, target_h):
            canvas = Image.new("RGBA", (target_w, target_h), bgcolor)
            # center the frame
            offset = ((target_w - f.width) // 2, (target_h - f.height) // 2)
            canvas.paste(f, offset, mask=f)
            composed = canvas
        else:
            # ensure background
            canvas = Image.new("RGBA", (target_w, target_h), bgcolor)
            canvas.paste(f, (0, 0), mask=f)
            composed = canvas
        # Convert to P mode later; but keep RGBA for safe alpha composition now
        composed_frames.append(composed)

    # Optionally create ping-pong effect
    if ping_pong and len(composed_frames) > 1:
        composed_frames = composed_frames + composed_frames[-2:0:-1]  # exclude last then reverse middle ones

    # Convert to palette mode for GIF (Pillow does this with .convert('P', palette=Image.ADAPTIVE))
    paletted_frames: list[Image.Image] = []
    for i, f in enumerate(composed_frames):
        # Convert to P mode with adaptive palette; convert() handles dithering if needed
        p = f.convert("P", palette=Image.ADAPTIVE)
        paletted_frames.append(p)

    # Save as animated GIF
    out_dir = os.path.dirname(output_path)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    first, *rest = paletted_frames
    if verbose:
        print(f"Saving {len(paletted_frames)} frames -> {output_path} (duration={duration_ms}ms, loop={'∞' if loop==0 else loop})")

    save_kwargs = dict(save_all=True, append_images=rest, duration=duration_ms, loop=loop, optimize=True)
    # disposal=2 may help some viewers; Pillow accepts 'disposal' in newer versions but it's optional
    try:
        first.save(output_path, format="GIF", **save_kwargs)
    except TypeError:
        # older Pillow may not accept optimize kw or others; attempt minimal save
        first.save(output_path, format="GIF", save_all=True, append_images=rest, duration=duration_ms, loop=loop)

    if verbose:
        print("Done.")

# Example usage:
if __name__ == "__main__":
    # Suppose you have 10 files: 0.svg ... 9.svg
    svg_sequence_to_gif(
        n=95,
        output_path="out/animation.gif",
        input_pattern="/Users/daniel.toby/Desktop/output/{i}.svg",
        start_index=0,
        duration_ms=80,
        loop=0,
        scale=1.0,
        ping_pong=False,
        verbose=True,
    )
