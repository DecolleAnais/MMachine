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
out vec4 vCameraPos;
out vec3 vWorldPos;
out vec2 vTextCoord; // texture
out vec3 vNormal; // normale

void main( )
{
    // position apres rotation et scale
    vec4 position_scaled = rotation_scale * vec4(position, 1.0);

    // obligation du vertex shader : calculer les coordonnées du sommet dans le repère projectif homogene de la camera
    gl_Position = mvpMatrix * position_scaled; 
    vCameraPos = modelMatrix * viewMatrix * vec4(position, 1.0);

    vTextCoord = textcoord;
    vNormal = normal;

    vec4 vWorldPosLocal = modelMatrix * position_scaled;
    vWorldPos = vWorldPosLocal.xyz;

    // lumiere
    // normale de la surface, dans le repere monde
    vec3 n = mat3(modelMatrix) * normal;

    // position de la camera dans le repere du monde
    vec4 sourceh = viewInvMatrix * vec4(0, 0, 0, 1);
   
    // rappel : mat4 * vec4 = vec4 homogene, pour retrouver le point / la direction reelle, il faut diviser par la 4ieme composante
    vec3 source = sourceh.xyz / sourceh.w;
     
    // direction entre le sommet et la source de lumiere
    vec3 l = source - position; 
    // OU ?
    //vec3 l = source - (position_scaled.xyz / position_scaled.w);
    
    // calculer le cosinus de l'angle entre les 2 directions, a verifier...
    cos_theta = max(0, dot(normalize(n), normalize(l)));

}
#endif



#ifdef FRAGMENT_SHADER

out vec4 vTextColor;

in vec2 vTextCoord; 
in vec3 vNormal;
in vec3 vWorldPos;
in vec4 vCameraPos;
in float cos_theta;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float fRenderHeight;
uniform float fMaxTextureU;
uniform float fMaxTextureV;

void main( )
{
    vec3 vNormalized = normalize(vNormal);

    vTextColor = vec4(0.0);

    float fScale = vWorldPos.y / fRenderHeight;

    const float fRange1 = 0.15f;
    const float fRange2 = 0.3f;
    const float fRange3 = 0.65f;
    const float fRange4 = 0.85f;

    if(fScale >= 0.0 && fScale <= fRange1) {
        vTextColor = texture(texture0, vTextCoord);
    }else if(fScale <= fRange2) {
        fScale -= fRange1;
        fScale /= (fRange2 - fRange1);

        float fScale2 = fScale;
        fScale = 1.0 - fScale;

        vTextColor += texture(texture0, vTextCoord) * fScale; 
        vTextColor += texture(texture1, vTextCoord) * fScale2; 
    }else if(fScale <= fRange3) {
        vTextColor = texture(texture1, vTextCoord);
    }else if(fScale <= fRange4) {
        fScale -= fRange3;
        fScale /= (fRange4 - fRange3);

        float fScale2 = fScale;
        fScale = 1.0 - fScale;

        vTextColor += texture(texture1, vTextCoord) * fScale; 
        vTextColor += texture(texture2, vTextCoord) * fScale2; 
    }else {
        vTextColor = texture(texture2, vTextCoord);
    }

    vTextColor *= cos_theta;

    // ignore tout ce qui precede
    vTextColor = vec4(1, 1, 1, 1) * floor(cos_theta * 5) / 5;
}
#endif