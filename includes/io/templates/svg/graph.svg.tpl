<svg xmlns="http://www.w3.org/2000/svg" width="{{ width }}" height="{{ height }}">
  <metadata>
    <generator name="libgsp" author="Mohammad Raziei" site="https://mohammadraziei.github.io/libgsp" />
    <libgsp version="{{ version }}" language="cpp" os="linux" />
    <content format="svg" tags="plot graph signal bar"/>
    <plotinfo signalscale="{{ signalscale }}" nodespacescale="{{ nodespacescale }}" />
  </metadata>
  <title>{{ title }}</title>
  <style>
    .node { fill: #00BCE3; stroke: black; stroke-width: 0.7; opacity: 0.8 }
    .node-label { font-size: 12 }
    .signal { stroke: red; fill: red; stroke-width: 2; }
    circle.signal { r: 1; }
    .signal-text { fill: red; }
  </style>

  {% for edge in edges %}
  <line class="edge"
        x1="{{ edge.x1 }}" y1="{{ edge.y1 }}"
        x2="{{ edge.x2 }}" y2="{{ edge.y2 }}"
        name1="{{ edge.name1 }}" name2="{{ edge.name2 }}"
        weight="{{ edge.weight }}" stroke="black" stroke-width="1.5"/>
  {% endfor %}

  {% for node in nodes %}
  <circle class="node" cx="{{ node.x }}" cy="{{ node.y }}" r="8"
          signal="{{ node.signal }}" coord="{{ node.coord }}"
          name="{{ node.name }}" />
  <text class="node-label" text-anchor="middle" dominant-baseline="middle"
        x="{{ node.x }}" y="{{ node.y }}">{{ node.name }}</text>

  <line class="signal" x1="{{ node.x }}" y1="{{ node.y }}"
        x2="{{ node.x }}" y2="{{ node.y_signal }}" />
  <circle class="signal" cx="{{ node.x }}" cy="{{ node.y_signal }}" r="1" />
  <text class="signal-text" text-anchor="middle" dominant-baseline="middle"
        font-size="6px" x="{{ node.x }}" y="{{ node.text_y }}">
    {{ node.signal }}
  </text>
  {% endfor %}
</svg>
