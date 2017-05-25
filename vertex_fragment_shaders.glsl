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
out vec2 vTextCoord; // texture
out vec3 vNormal; // normale
out vec3 source;
out vec4 vPositionMVP;
out vec4 vPositionM;


void main( )
{
    // obligation du vertex shader : calculer les coordonnées du sommet dans le repère projectif homogene de la camera
    vPositionM = modelMatrix * vec4(position, 1);
    vPositionMVP = mvpMatrix * vec4(position, 1);
    gl_Position = vPositionMVP; 

    // Transfert des coordonnées de texture et des normales au fragment shader
    vTextCoord = textcoord;
    vNormal = mat3(normalMatrix) * normal;

    // position de la camera dans le repere du monde
    //vec4 sourceh = viewInvMatrix * vec4(0, 0, 0, 1);
    vec4 sourceh = vec4(0,0,50,1);

    // rappel : mat4 * vec4 = vec4 homogene, pour retrouver le point / la direction reelle, il faut diviser par la 4ieme composante
    source = sourceh.xyz / sourceh.w;
}
#endif



#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

in vec2 vTextCoord; 
in vec3 vNormal;
in vec4 vCameraPos;
in vec3 source;
in vec4 vPositionM;
in vec4 vPositionMVP;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec3 spotP1Pos;
uniform vec3 spotP2Pos;
uniform vec3 spotP1Dir;
uniform vec3 spotP2Dir;

void main( )
{
    // Spotlights
    int on = 1;
    vec4 spotLight = vec4(0.0, 0.0, 0.0, 0.0);

    if(on == 1){
        // Parameters & vertex position in world reference
        float coneCos = 0.85;
        float attenuation = 0.4;
        vec3 pos = vPositionM.xyz / vPositionM.w;

        // Useful vars
        float dist, cos, dif, factor;
        vec3 lSpot;

        // PLAYER 1 SPOTLIGTH'S
        vec4 spotLightP1 = vec4(0.0, 0.0, 0.0, 0.0);

        dist = distance(pos, spotP1Pos);
        lSpot = normalize(pos - spotP1Pos);

        cos = dot(spotP1Dir, lSpot);
        dif = 1.0 - coneCos;
        factor = clamp((cos - coneCos)/dif, 0.0, 1.0);

        if(cos > coneCos)
            spotLightP1 = vec4(1.0, 1.0, 1.0, 1.0) * factor/(dist * attenuation);

        // PLAYER 2 SPOTLIGTH'S
        vec4 spotLightP2 = vec4(0.0, 0.0, 0.0, 0.0);

        dist = distance(pos, spotP2Pos);
        lSpot = normalize(pos - spotP2Pos);

        cos = dot(spotP2Dir, lSpot);
        dif = 1.0 - coneCos;
        factor = clamp((cos - coneCos)/dif, 0.0, 1.0);

        if(cos > coneCos)
            spotLightP2 = vec4(1.0, 1.0, 1.0, 1.0) * factor/(dist * attenuation);

        // SUM

        spotLight = spotLightP1 + spotLightP2;
    }


    // Ambiant light
    vec3 l = source - vPositionMVP.xyz / vPositionMVP.w;;
    // calculer le cosinus de l'angle entre les 2 directions, a verifier...
    float cos_theta = max(0, dot(normalize(vNormal), normalize(l)));
    vec4 ambiantLight = vec4(1.0, 1.0, 1.0, 1.0) * 0.1 * cos_theta;

    // Light on pixel
    vec4 localLight = (vec4(0.2, 0.2, 0.2, 0.2) * cos_theta) + spotLight;

    // Colors
    vec4 grassColor = texture(texture1, vTextCoord) * texture(texture0, vTextCoord*30);
    vec4 roadColor = (1-texture(texture1, vTextCoord)) * texture(texture2, vTextCoord*30);
    vec4 textureColor = grassColor + roadColor;
    

    
    fragment_color = textureColor * (ambiantLight + spotLight);
}
#endif