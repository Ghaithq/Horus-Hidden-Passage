# Horus-Hidden-Passage
<p align="center">
  <img src="https://github.com/user-attachments/assets/d0bda74d-831a-4321-bbe9-d1f179e80550">
</p>

## Description
Driven by curiosity, an unsuspecting explorer, ventures into the depths of
an ancient Egyptian tomb, unaware of its dark secret. But as fate would have it, his
exploration turns sinister when he becomes trapped within its ancient confines. With each
breath, his air supply diminishes, and as darkness descends, Hamza realizes that the
shadows conceal more than just his confinement. To his horror, he discovers he is not
alone in this suffocating darkness, and that survival is his ultimate goal.

## Game Walkthrough
1) The player is trapped inside a tomb and must find 5 hidden pieces to escape.
2) Multiple obstacles will get in the player's way while searching, he needs to light all the torches for better visibility.
3) Elements within the tomb, such as light and visual distortions, will hinder the player.
4) The player must manage their air supply and escape as quickly as possible before suffocating.
5) If the player finds all 5 pieces and escapes before running out of air, the tomb's ceiling will rise and the door will open. Otherwise, he loses.

## Implementation
### System Diagram
The following diagram shows the relationship between the classes (and structs) in the project:
![image](https://github.com/user-attachments/assets/57179120-164f-4eb0-b9bb-0f42c520e4cb)

### Engine Parts
#### 1) Shader Program
In our engine, the shader program defines the vertex and fragment shaders that run on the GPU during
rendering. It will contain an OpenGL program object.

#### 2) Mesh
The mesh is a collection of vertices and faces. In OpenGL, we define a mesh using 3 OpenGL objects:
<pre>
<strong>Vertex Buffer</strong>: This is a buffer that contains the data of all the vertices.
  
<strong>Element Buffer</strong>: This is a buffer that contains data that defines the indices of the 3 vertices
needed to draw each triangle (or 2 if lines or 1 if points).

<strong>Vertex Array</strong>: This is an object that defines how the data in the vertex buffer is interpreted and
sent to the vertex shader as attributes.
</pre>

#### 3) Transform
To draw different instances of a mesh at different positions, orientations, and/or scales, we send a
transformation matrix to the vertex shader as a uniform. The transformation matrix is used to transform
the vertex positions from the object's local space to the world space.

#### 4) Pipeline State
OpenGL is a state machine where the options we pick are stored in the OpenGL context and affect the
upcoming draw calls. Since each object may require different options while drawing (e.g. transparent
objects require blending while Opaque objects don't), we would need to store the options for each object
in a data structure and set the OpenGL options to match the given options before drawing.
This is where we use the "PipelineState" structure which we will use to store the depth testing, face culling,
blending, and color/depth mask options. The setup function of the PipelineState sets the OpenGL options
to match the ones stored in the corresponding PipelineState instance.

#### 5) Texture
A 2D Texture is a sampleable storage containing a 2D array of pixels. By "sampleable", we mean that we can
sample a color from it in the shaders.

#### 6) Sampler
There is more than one way to sample a texture. For example, we can choose between nearest or linear
filtering and we can select between different wrapping options. A `sampler` is an OpenGL object that can
store the sampling options to use while sampling a texture.

#### 7) Material
We will combine all the previous parts using one class which we will call `Material`.
Since there are lots of different types of Materials, we chose to create a base class for all materials (which
we call `Material`) and we inherit from it to create more specific materials (e.g. `TintedMaterial` and
`TexturedMaterial`).

#### 8) Entities and Components
An entity-component-system `ECS` framework consists of 3 parts:
<pre>
<strong>Entities</strong>: These are containers that contain a set of components. An entity does nothing more than
being a simple container and ideally, it should contain no logic of its own.
  
<strong>Components</strong>: which are data objects that can be added to entities. The roles and the data of the
entities are defined by their components. For example, if an entity contains a camera component,
then it is a camera and we should be able to find camera-specific data (e.g. field of view angle) inside
the camera component.
  
<strong>Systems</strong>: which defines the logic. For example, if we want to render a set of entities, we should
implement a renderer system that draws the entities in every frame.
</pre>

#### 9) Forward Renderer System
The forward renderer will handle the transparent objects by sorting them by the distance along
the camera's forward direction. The draw order of transparent objects is from far to near.

#### 10) Sky Rendering
Here, we will modify the `ForwardRenderer` system class to draw a sky sphere around the camera.

#### 11) Postprocessing
Sometimes, we want to apply an image effect on the rendered scene. This is called postprocessing which
can be applied by rendering the scene to a texture and then rendering the texture to the screen using the
effect shader.

## Screenshots
![image](https://github.com/user-attachments/assets/0d5ca402-10c4-4453-a62f-29df6ff0787d)
![image](https://github.com/user-attachments/assets/71dfb2eb-7388-4df6-b47c-66c7bbdd3fbe)
![image](https://github.com/user-attachments/assets/bd444cfc-a831-460a-a330-172d50a2d89b)
![image](https://github.com/user-attachments/assets/d3d82fd3-1ece-4cac-928e-c720f3b21df9)
![image](https://github.com/user-attachments/assets/3c3ac84f-c1a5-4037-8328-0b65d382df41)
![image](https://github.com/user-attachments/assets/20a3e46d-207c-4524-8a24-93ec54ec549d)
![image](https://github.com/user-attachments/assets/3425c0c4-29b4-41ef-a39d-4e5a090c8af1)
![image](https://github.com/user-attachments/assets/465197b8-98ec-4e22-b5f3-d946c7f82b12)
![image](https://github.com/user-attachments/assets/9d2b78b6-20dc-4bdf-be38-e97b4bd895b9)

## ©️Developers

| Name                 |         Email          |
|----------------------|:----------------------:|
| Fares Atef           | faresatef553@gmail.com |
| Ghaith Mohamed       |  gaoia123@gmail.com    |
| Amr ElSheshtawy      | Sheshtawy321@gmail.com |
| Amr Magdy            |  amr4121999@gmail.com  |
