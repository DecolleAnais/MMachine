#version 330

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 position;    // coordonnees du sommet
layout(location = 1) in vec2 textcoord;     // coordonnees de texture du sommet
layout(location = 2) in vec3 normal;     // normale du sommet

uniform mat4 mvpMatrix;    // matrice passage du repere local au repere projectif homogene de la camera
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 viewInvMatrix;

uniform mat4 rotation_scale;

out float cos_theta;
//out vec2 vertex_textcoord; // utile pour une texture

void main( )
{
    // position apres scale
    vec4 position_scaled = vec4(position, 1) * rotation_scale;

    // obligation du vertex shader : calculer les coordonnées du sommet dans le repère projectif homogene de la camera
    gl_Position = mvpMatrix * position_scaled;
     
    // normale de la surface, dans le repere monde
    vec3 n = mat3(modelMatrix) * normal;
   
    // position de la camera dans le repere du monde
    vec4 sourceh = viewInvMatrix * vec4(0, 0, 0, 1) * position_scaled;
   
    // rappel : mat4 * vec4 = vec4 homogene, pour retrouver le point / la direction reelle, il faut diviser par la 4ieme composante
    vec3 source = sourceh.xyz / sourceh.w;
     
    // direction entre le sommet et la source de lumiere
    vec3 l = source - position;
    // calculer le cosinus de l'angle entre les 2 directions, a verifier...
    cos_theta = max(0, dot(normalize(n), normalize(l)));
     
    // vertex_textcoord = textcoord; // utile pour une texture
}
#endif

#ifdef FRAGMENT_SHADER
// recuperer l'orientation calculee dans le vertex shader... a completer

out vec4 fragment_color;
in float cos_theta;
// in vec2 vertex_textcoord; // utile pour une texture

uniform vec4 color;
//uniform sampler2D texture0; // utile pour une texture

void main( )
{
    // utiliser l'orientation pour modifier la couleur de base... a completer
    fragment_color = color * floor(cos_theta * 5) / 5;    // orange opaque

    // avec texture
    // vec4 color_texture = texture(texture0, vertex_textcoord);
    // fragment_color = color_texture * floor(cos_theta * 5) / 5;
}
#endif