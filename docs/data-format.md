# SimViewer Data Format

This document defines the directory layout, YAML metadata, and binary payloads accepted by SimViewer.

## Directory Layout

Each dataset is a directory with this structure:

```text
simulation_output/
  description.yaml
  frame_count.txt
  results/
    0/
    1/
    2/
    ...
```

Files and directories have the following meaning:

- `description.yaml`: scene metadata and object declarations
- `frame_count.txt`: total number of frames as a decimal integer
- `results/<frame>/`: one directory per frame, numbered from `0`
- `results/<frame>/<Name>.out`: binary payload for the object named `Name`

`frame_count.txt` is optional. If it is missing, SimViewer assumes a single-frame dataset.

## `description.yaml`

`description.yaml` is a YAML document with the following top-level fields.

| Field | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `Dimension` | integer | no | `3` | Scene dimension. Supported values are `2` and `3`. |
| `Radius` | float | no | `1.0` | Initial camera scale in 2D or orbit radius in 3D. |
| `Objects` | sequence | yes | none | Ordered list of object declarations. |

### Example

```yaml
Dimension: 3
Radius: 0.5
Objects:
  - Name: surface
    Animated: true
    Indexed: true
    Primitive: Triangles
    Material:
      Albedo: [0, 0, 1, 1]
```

## Object Schema

Each entry in `Objects` defines one renderable object.

| Field | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `Name` | string | yes | none | Object identifier. The viewer loads `results/<frame>/<Name>.out`. |
| `Primitive` | string | no | `Triangles` | One of `Points`, `Lines`, or `Triangles`. |
| `Shader` | string | no | `default` | Shader family. Public values are `default`, `heatmap`, and `fracmap`. |
| `Indexed` | bool | no | `false` | Whether the binary payload contains an index buffer. |
| `Animated` | bool | no | `false` | Whether the viewer loads additional frames beyond frame `0`. |
| `TopoFixed` | bool | no | `false` | Whether topology payloads are stored only in the initial frame. |
| `HeatDiv` | bool | no | `false` | Heatmap mode flag. `false` selects a sequential heatmap, `true` selects a diverging heatmap. |
| `Material` | map | no | see below | Initial material settings for the object. |

### `Material`

| Field | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `Mode` | string | no | `Opaque` | Blend mode. One of `Opaque`, `Cutout`, `Transparent`, `Fade`. |
| `Albedo` | float[4] | no | `[1, 1, 1, 1]` | RGBA base color. |
| `Metallic` | float | no | `0.0` | Metallic parameter for applicable shaders. |
| `Roughness` | float | no | `0.5` | Roughness parameter for applicable shaders. |
| `Visible` | bool | no | `true` | Initial object visibility. |

## Shader Resolution

The `Shader` field selects a shader family. SimViewer combines that family with `Dimension` and `Primitive` to choose a built-in renderer variant.

Supported public shader families are:

- `default`
- `heatmap`
- `fracmap`

The resulting attribute set is inferred automatically:

- positions are always present
- normals are present in 3D
- heat scalars are present for `heatmap` and `fracmap`
- radii are associated with point primitives

Texture coordinates exist in internal viewer data structures but are not part of the documented public dataset contract.

## Binary Encoding

Each object frame is stored in a separate binary file named `<Name>.out`.

### Scalar Types

- `uint32`: counts and indices
- `float32`: positions, normals, heat values, radii, and material-related scalar payloads

The documented on-disk format uses little-endian encoding.

### Common Layout Rules

Every `.out` file begins with:

1. `vertex_count` as `uint32`

The viewer then reads attribute arrays in this order:

1. positions
2. normals, when `Dimension == 3`
3. heat values, when `Shader` is `heatmap` or `fracmap`
4. texture coordinates, only for internal formats not covered by this public specification
5. radii, for point primitives
6. `index_count`, when `Indexed == true`
7. indices, when `Indexed == true`

If `TopoFixed: true`, topology payloads are expected only in frame `0`. In practice this means later frames omit index buffers and any topology-scoped attribute blocks, including point radii when those radii are treated as fixed object geometry.

## Public Binary Layouts

The following layouts are the supported public object families emitted by the current generator code and accepted by SimViewer.

### 2D Indexed Lines

Used for contour-style line meshes.

Binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec2<float32>`
3. `index_count: uint32`
4. `indices: index_count * uint32`

### 2D Heatmap Points

Used for particle-style point clouds with scalar values.

Binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec2<float32>`
3. `heats: vertex_count * float32`
4. `radii: vertex_count * float32`

### 2D Default Points

Used for point clouds without heat values.

Binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec2<float32>`
3. `radii: vertex_count * float32`

### 2D Heatmap Lines

Used for line segments with one scalar value per vertex.

Binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec2<float32>`
3. `heats: vertex_count * float32`

This layout is typically used with non-indexed line pairs, where each consecutive pair of vertices forms one segment.

### 3D Indexed Triangles

Used for surface meshes.

Binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec3<float32>`
3. `normals: vertex_count * vec3<float32>`
4. `index_count: uint32`
5. `indices: index_count * uint32`

### 3D Heatmap Indexed Triangles

Used for surface meshes with one scalar value per vertex.

Binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec3<float32>`
3. `normals: vertex_count * vec3<float32>`
4. `heats: vertex_count * float32`
5. `index_count: uint32`
6. `indices: index_count * uint32`

### 3D Default Points

Used for point clouds with normals.

Current binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec3<float32>`
3. `normals: vertex_count * vec3<float32>`

Extended binary layout:

1. `vertex_count: uint32`
2. `positions: vertex_count * vec3<float32>`
3. `normals: vertex_count * vec3<float32>`
4. `radii: vertex_count * float32`

When radii are not written, the viewer still accepts the payload and falls back to renderer behavior for missing point-size data.

## Frame Semantics

- If `Animated: false`, SimViewer loads only frame `0`.
- If `Animated: true`, SimViewer loads frame `0` immediately and continues loading subsequent frames in the background.
- If `Indexed: true`, index data is read after vertex attributes.
- If `TopoFixed: true`, topology payloads are read from frame `0` and reused for later frames.
- Heatmap ranges are accumulated across loaded frames and drive the shader normalization used by the object.

## Naming and Numbering Rules

- Frame directories are decimal integers starting at `0`.
- Object file names must match `Name` exactly, with the `.out` extension appended.
- Object declarations are ordered; the viewer preserves that order in the object list and UI.

## Reference Examples

### 2D Contour Scene

```yaml
Dimension: 2
Radius: 0.0768
Objects:
  - Name: contour
    Animated: true
    Indexed: true
    Primitive: Lines
    Material:
      Albedo: [1, 1, 1, 1]
```

### 2D Particle Scene

```yaml
Dimension: 2
Radius: 0.505
Objects:
  - Name: regular
    Animated: true
    Primitive: Points
    Shader: heatmap
  - Name: boundary
    Primitive: Points
    Material:
      Albedo: [0.5, 0.5, 0.5, 1]
  - Name: velocity
    Animated: true
    Primitive: Lines
    Shader: heatmap
```

### 3D Surface Scene

```yaml
Dimension: 3
Radius: 0.5
Objects:
  - Name: surface
    Animated: true
    Indexed: true
    Primitive: Triangles
    Material:
      Albedo: [0, 0, 1, 1]
```

### 3D Heatmap Surface Scene

```yaml
Dimension: 3
Radius: 0.2
Objects:
  - Name: curv
    Animated: true
    Indexed: true
    Shader: heatmap
    Primitive: Triangles
  - Name: mag_pressure
    Animated: true
    Indexed: true
    Shader: heatmap
    Primitive: Triangles
```
