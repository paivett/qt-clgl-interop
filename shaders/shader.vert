#version 450

uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 p_matrix;

in vec4 vertex_coord;

out vec4 frag_coord;

void main() {
    mat4 mv_matrix = v_matrix * m_matrix;
    
    frag_coord = m_matrix * vec4(vertex_coord.xyz, 1.0);

    // Calculate vertex position in screen space
    gl_Position = p_matrix * mv_matrix * vec4(vertex_coord.xyz, 1.0);
}
