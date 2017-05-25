#version 330

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 position;    // coordonnees du sommet
layout(location = 1) in vec2 textcoord;     // coordonnees de texture du sommet
layout(location = 2) in vec3 normal;     // normale du sommet

uniform mat4 mvpMatrix;    // matrice passage du repere local au repere projectif homogene de la camera
uniform mat4 mvMatrix;
uniform mat4 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 viewInvMatrix;

out vec4 vCameraPos;
out vec3 vWorldPos;
out vec2 vTextCoord; // texture
out vec3 vNormal; // normale
out vec3 source;
out vec4 vPosition;


void main( )
{
    // obligation du vertex shader : calculer les coordonnées du sommet dans le repère projectif homogene de la camera
    vPosition = mvpMatrix * vec4(position, 1);
    gl_Position = vPosition; 

    // vCameraPos = modelMatrix * viewMatrix * vec4(position, 1.0);

    vTextCoord = textcoord;
    vNormal = mat3(normalMatrix) * normal;

    // vec4 vWorldPosLocal = modelMatrix * position_scaled;
    // vWorldPos = vWorldPosLocal.xyz;

    // lumiere
    // normale de la surface, dans le repere monde
    // vec3 n = mat3(viewMatrix * modelMatrix) * normal;

    // position de la camera dans le repere du monde
    vec4 sourceh = viewInvMatrix * vec4(0, 0, 0, 1);
   
    // rappel : mat4 * vec4 = vec4 homogene, pour retrouver le point / la direction reelle, il faut diviser par la 4ieme composante
    source = sourceh.xyz / sourceh.w;
     
    // direction entre le sommet et la source de lumiere
    // vec3 l = source - position;
    
    // // calculer le cosinus de l'angle entre les 2 directions, a verifier...
    // cos_theta = max(0, dot(normalize(n), normalize(l)));
}
#endif



#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

in vec2 vTextCoord; 
in vec3 vNormal;
in vec3 vWorldPos;
in vec4 vCameraPos;
in vec3 source;
in vec4 vPosition;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
// uniform float fRenderHeight;
// uniform float fMaxTextureU;
// uniform float fMaxTextureV;

void main( )
{
    vec3 l = source - vPosition.xyz / vPosition.w;
    
    // calculer le cosinus de l'angle entre les 2 directions, a verifier...
    float cos_theta = max(0, dot(normalize(vNormal), normalize(l)));
    //fragment_color = vec4(1, 1, 1, 1) * cos_theta;
    vec4 grassColor = texture(texture1, vTextCoord) * texture(texture0, vTextCoord*30);
    vec4 roadColor = (1-texture(texture1, vTextCoord)) * texture(texture2, vTextCoord*30);

    vec4 textureColor = grassColor + roadColor;
    
    fragment_color = textureColor * cos_theta;



    // TESTS TEXTURE
    /*vec3 vNormalized = normalize(vNormal);

    fragment_color = vec4(0.0);

    float fScale = vWorldPos.y / fRenderHeight;

    const float fRange1 = 0.15f;
    const float fRange2 = 0.3f;
    const float fRange3 = 0.65f;
    const float fRange4 = 0.85f;

    if(fScale >= 0.0 && fScale <= fRange1) {
        fragment_color = texture(texture0, vTextCoord);
    }else if(fScale <= fRange2) {
        fScale -= fRange1;
        fScale /= (fRange2 - fRange1);

        float fScale2 = fScale;
        fScale = 1.0 - fScale;

        fragment_color += texture(texture0, vTextCoord) * fScale; 
        fragment_color += texture(texture1, vTextCoord) * fScale2; 
    }else if(fScale <= fRange3) {
        fragment_color = texture(texture1, vTextCoord);
    }else if(fScale <= fRange4) {
        fScale -= fRange3;
        fScale /= (fRange4 - fRange3);

        float fScale2 = fScale;
        fScale = 1.0 - fScale;

        fragment_color += texture(texture1, vTextCoord) * fScale; 
        fragment_color += texture(texture2, vTextCoord) * fScale2; 
    }else {
        fragment_color = texture(texture2, vTextCoord);
    }

    fragment_color *= cos_theta;*/

}
#endif