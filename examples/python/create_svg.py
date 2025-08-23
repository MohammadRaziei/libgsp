import math
import cairosvg
from PIL import Image
import io



def create_svg_graph_signal(E, coords, signals):
    """
    Generate SVG graph with nodes, edges, and signal indicators
    
    Parameters:
        E: List of edges as tuples (source_index, target_index)
        coords: List of (x, y) tuples for node positions
        signals: Dictionary {node_index: (value, angle)} where:
            - value: Signal magnitude (positive/negative)
            - angle: Direction angle in degrees
            
    Returns:
        SVG XML string
    """

    signalscale = 100

    nodespacescale = 100

    title="Network"

    # Helper function to convert degrees to radians
    signal_fontsize = 6

    min_x = float("inf")
    max_x = -float('inf')
    min_y = float("inf")
    max_y = -float('inf')

    for idx, coord in enumerate(coords):
        x = nodespacescale*coord[0]
        y = nodespacescale*coord[1]
        x_sig = x
        y_sig = y + signalscale*signals[idx]

        x_l_min = min(x, x_sig) - 20
        x_l_max = max(x, x_sig) + 20

        
        y_l_min = min(y, y_sig) - 20
        y_l_max = max(y, y_sig) + 20

        if x_l_min < min_x:
            min_x = x_l_min
        if x_l_max > max_x:
            max_x = x_l_max
        if y_l_min < min_y:
            min_y = y_l_min
        if y_l_max > max_y:
            max_y = y_l_max


    libgsp_version = "0.0.0"


    metadata = '<generator name="libgsp" author="Mohammad Raziei" site="https://mohammadraziei.github.io/libgsp" />'
    metadata += f'<libgsp version="{libgsp_version}" language="cpp" os="linux"/>'
    metadata += f'<content format="svg" tags="plot graph signal bar"/>'
    metadata += f'<plotinfo signalscale="{signalscale}" nodespacescale="{nodespacescale}"/>'
    
    # SVG template
    svg = [
        '<svg xmlns="http://www.w3.org/2000/svg" width="%i" height="%i">'%(max_x-min_x, max_y-min_y),
        '  <metadata>%s</metadata>'%(metadata),
        '  <title>%s</title>'%(title),
        '  <style>',
        '    .node { fill: #00BCE3; stroke: black; stroke-width: 0.7; opacity: 0.8}',
        '    .node-label { font-size: 12; text-anchor:middle; dominant-baseline:middle; }'
        '    .signal { stroke: red; fill: red; stroke-width: 2;}',
        '    circle.signal { r: 1; }',
        '    .signal-text { fill: red; }',
        '  </style>'
    ]


    # Draw edges
    for e in E:
        src = e[0]
        tgt = e[1]
        w = e[2] if len(e)>2 else 1
        x1, y1, z1 = coords[src]
        x2, y2, z2 = coords[tgt]
        svg.append(f'  <line class="edge" x1="{x1*nodespacescale-min_x}" y1="{-y1*nodespacescale+max_y}" x2="{x2*nodespacescale-min_x}" y2="{-y2*nodespacescale+max_y}" v1="{src}" v2="{tgt}" weight="{w}" stroke="black" stroke-width="1.5"/>')

    # Draw nodes and signals
    for idx, coord in enumerate(coords):   
        signal = signals[idx]

        x =  coord[0]*nodespacescale-min_x
        y = -coord[1] * nodespacescale+max_y

        nodename = f"v{idx}" # for example for ungiven names 
        # Node circle
        svg.append(f'  <circle class="node" cx="{x}" cy="{y}" r="8" sig="{signal}" vx="{coord[0]}" vy="{coord[1]}" vz="{coord[2]}" v="{idx}" name="{nodename}"/>')
        
        # Node label
        svg.append(f'  <text class="node-label" x="{x}" y="{y}">{nodename}</text>')
        
        # Signal
        if signal is not None:

            value = -signal * signalscale  # Scaling factor
            

            
            # Signal line
            svg.append(f'  <line class="signal" x1="{x}" y1="{y}" x2="{x}" y2="{y+value}"/>')
            
            # Signal arrowhead
            svg.append(f'  <circle class="signal" cx="{x}" cy="{y+value}" r="1"/>')
            
            # Signal label
            dy = -signal_fontsize if value < 0 else signal_fontsize # Inverted Y-axis in SVG
            text_x = x
            text_y = y + value + dy
            svg.append(f'  <text class="signal-text" text-anchor="middle" dominant-baseline="middle" font-size="{signal_fontsize}px" x="{text_x}" y="{text_y}">{signal:g}</text>')

    svg.append('</svg>')
    return '\n'.join(svg)

# Example usage
if __name__ == "__main__":
    # Graph definition
    E = [(0,1), (0,2), (1,2), (2,3)]  # Edges as index pairs
    coords = [
        (0, 0, 0),   # Node 1
        (2, 0, 0),   # Node 2
        (1, -1, 0),   # Node 3
        (3, -1, 0)    # Node 4
    ]
    signals = [
        -0.04,  # Node 1
        0.31,     # Node 2
        0.06,     # Node 3
        0.39,      # Node 4
    ]

    # Create SVG content
    svg_content = create_svg_graph_signal(E, coords, signals)

    with open("graph.svg", 'w') as f:
        f.write(svg_content)

    # Save to temporary file and display with Matplotlib

    png_bytes = cairosvg.svg2png(bytestring=svg_content, scale=10)

    # Open and display the PNG using Pillow
    image = Image.open(io.BytesIO(png_bytes))
    image.show()

