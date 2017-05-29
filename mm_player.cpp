#include "src/terrain.hpp"
#include "src/controller.hpp"
#include "src/player.hpp"
#include "src/scoreManager.hpp"
#include "src/parser.hpp"
#include "src/objectsManager.hpp"

#include "mat.h"
#include "wavefront.h"
#include "orbiter.h"
#include "draw.h"
#include "app.h"
#include "texture.h"

#include "program.h"
#include "uniforms.h"

#include <chrono>
#include <cmath>

typedef std::chrono::high_resolution_clock Clock;

class Play : public App
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    Play( ) : 
      App(1024, 640), 
      controller1_(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT),
      controller2_('z', 's', 'q', 'd'),
      terrain_(Point(-20.f, -20.f, 0.f), Point(20.f, 20.f, 0.f)),
      objectsManager_(),
      score_(4 , 2, terrain_.getCheckpoints())
    {}
    
    int init( )
    {
        /************* INIT MESH VEHICULES *************/
        vehicule1_ = read_mesh("MMachine/data/obj/mmachine.obj") ;
        vehicule1_.default_color(Color(1.0f, 0.f, 0.f)) ;
        vehicule2_ = read_mesh("MMachine/data/obj/mmachine.obj") ;
        vehicule2_.default_color(Color(0.0f, 0.f, 1.f)) ;

        /************* INIT JOUEURS *************/
        joueur1_.set_terrain(&terrain_) ;
        joueur1_.set_controller(&controller1_) ;
        joueur2_.set_terrain(&terrain_) ;
        joueur2_.set_controller(&controller2_) ;
        
        joueur1_.setOtherPlayer(joueur2_);
        joueur2_.setOtherPlayer(joueur1_);

        joueur1_.setObjectsManager(&objectsManager_);
        joueur2_.setObjectsManager(&objectsManager_);

        joueur1_.spawn_at(Point(89.0,27.0,0), Vector(1,0,0)) ;
        joueur1_.activate() ;
        joueur2_.spawn_at(Point(89.0,25.0,0), Vector(1,0,0)) ;
        joueur2_.activate() ;

        oldPmin_ = Point(std::min(joueur1_.get_x(), joueur2_.get_x()),
                    std::min(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));
        oldPmax_ = Point(std::max(joueur1_.get_x(), joueur2_.get_x()),
                    std::max(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));

        /************* INIT BOITES ENGLOBANTES *************/
        Point p1, p2;
        vehicule1_.bounds(p1, p2);
        joueur1_.set_bounding_box(p1, p2);
        vehicule2_.bounds(p1, p2);
        joueur2_.set_bounding_box(p1, p2);

        /************* INIT TEXTURES *************/
        textures.resize(2);
        samplers.resize(2);

        textures[0] = read_texture(0, "MMachine/data/textures/grass.png");
        glGenSamplers(1, &samplers[0]);
        glSamplerParameteri(samplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers[0], GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(samplers[0], GL_TEXTURE_WRAP_T, GL_REPEAT);

        textures[1] = read_texture(1, "MMachine/data/textures/route.png");
        glGenSamplers(1, &samplers[1]);
        glSamplerParameteri(samplers[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers[1], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers[1], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(samplers[1], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        textures[2] = read_texture(2, "MMachine/data/textures/dirt.png");
        glGenSamplers(1, &samplers[2]);
        glSamplerParameteri(samplers[2], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(samplers[2], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(samplers[2], GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(samplers[2], GL_TEXTURE_WRAP_T, GL_REPEAT);

        /************* INIT SHADER *************/
        m_program = read_program("MMachine/vertex_fragment_shaders.glsl");
        program_print_errors(m_program);
        shadow_program = read_program("MMachine/shadow_shaders.glsl");
        program_print_errors(shadow_program);

        /************* INIT CONFIG OPENGL *************/
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest
        
        /* Init shadowmap */
        shadowWidth_ = 1024 * 4;
        shadowHeight_ = 1024 * 4;
        // Init buffer & configuration
        glGenFramebuffers(1, &shadowBuffer_);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer_);
            // Init texture & configuration
            glGenTextures(1, &shadowMap_);
            glBindTexture(GL_TEXTURE_2D, shadowMap_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowWidth_, 
                        shadowHeight_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            // GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
            // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap_, 0);

            glDrawBuffer(GL_NONE);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cerr << "Error in initialising shadowmap." << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        start_ = Clock::now();

        return 0;   // ras, pas d'erreur
    }
    

    int quit( )
    {
        release_program(m_program);
        vehicule1_.release();
        vehicule2_.release();
        glDeleteSamplers(1, &samplers[2]);
        glDeleteSamplers(1, &samplers[1]);
        glDeleteSamplers(1, &samplers[0]);
        glDeleteTextures(1, &textures[2]);   
        glDeleteTextures(1, &textures[1]);   
        glDeleteTextures(1, &textures[0]);
        terrain_.release();
        score_.release();
        objectsManager_.release();
        return 0;
    }
    
    
    Transform updateCamera(int score){
        /* Sélection des points décrivant la boite englobantes des joueurs à afficher */
        Point pminT, pmaxT;
        // Pas de premier, on centre la caméra entre les deux joueurs
        if(score == -1){
            pminT = Point(std::min(joueur1_.get_x(), joueur2_.get_x()),
                    std::min(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));
            pmaxT = Point(std::max(joueur1_.get_x(), joueur2_.get_x()),
                    std::max(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));
        }
        // Joueur 1 premier
        else if(score == 0){
            pminT = joueur1_.getPosition();
            pmaxT = joueur1_.getPosition();
        }
        // Joueur 2 premier
        else if(score == 1){
            pminT = joueur2_.getPosition();
            pmaxT = joueur2_.getPosition();
        }
        else
            throw std::invalid_argument("Parameter score is mean to be ranged in [-1; numLastPlayer - 1].");

        /* Calcul du déplacement de la caméra */
        Clock::time_point time = Clock::now();
        float delta = (float)std::chrono::duration_cast<std::chrono::milliseconds>(time - oldTime_).count() / 1000.0;
        oldTime_ = time;

        float coeffSpeed = 7.5;
        float pminXS = (pminT.x - oldPmin_.x) * coeffSpeed;
        float pminYS = (pminT.y - oldPmin_.y) * coeffSpeed;
        float pminZS = (pminT.z - oldPmin_.z) * coeffSpeed;
        float pmaxXS = (pmaxT.x - oldPmax_.x) * coeffSpeed;
        float pmaxYS = (pmaxT.y - oldPmax_.y) * coeffSpeed;
        float pmaxZS = (pmaxT.z - oldPmax_.z) * coeffSpeed;
        
        Point pmin, pmax;
        pmin.x = oldPmin_.x + pminXS * delta;
        pmin.y = oldPmin_.y + pminYS * delta;
        pmin.z = oldPmin_.z + pminZS * delta;
        pmax.x = oldPmax_.x + pmaxXS * delta;
        pmax.y = oldPmax_.y + pmaxYS * delta;
        pmax.z = oldPmax_.z + pmaxZS * delta;

        /* Calcul de la hauteur de la caméra */
        float dist = distance(pmin, pmax);
        Point centerCamera(center(pmin, pmax));
        float cameraDist = centerCamera.z + dist/10.0 + 5.0;
        Point cameraPos = centerCamera + Vector(0, 0, std::max(0.0f, cameraDist));

        /* Mise a jour des anciennes positions */
        oldPmin_ = pmin;
        oldPmax_ = pmax;

        // if(cameraPos.x != cameraPos.x || cameraPos.y != cameraPos.y){
        //     throw std::logic_error("Camera is not set right.");
        // }

        return Lookat(cameraPos, center(pmin, pmax), Vector(0, 1, 0));
    }


    void updateScores(){
        score_.updateCheckpoints(joueur1_.getPosition(), joueur2_.getPosition());
        bool falling_player_1 = joueur1_.isFallen();
        bool falling_player_2 = joueur2_.isFallen();
        if(falling_player_1) {
            score_.updateScore(1);  // si le joueur 1 est tombé, le joueur 2 gagne la manche
            winner_time_ = Clock::now();
        }else if(falling_player_2) {
            score_.updateScore(0);  // si le joueur 2 est tombé, le joueur 1 gagne la manche
            winner_time_ = Clock::now();
        // s'il y a un trop grand écart dans le nombre de checkpoints validés (triche) ou si les joueurs sont trop espacés, désignation d'un gagnant    
        }else if(score_.getEcartCheckpoints() > 4 || distance(joueur1_.getPosition(), joueur2_.getPosition()) >= 50){
            int first = score_.getFirst(joueur1_.getPosition(), joueur2_.getPosition());
            score_.updateScore(first);
            winner_time_ = Clock::now();
        }        
    }

    void updateScene(){
        /************* DEPLACEMENT JOUEURS *************/
        joueur1_.step();
        joueur2_.step();
    }

    void renderScene(const Transform& view, const Transform& projection){
        /************* DESSIN VEHICULES *************/
        draw(vehicule1_, joueur1_.transform(), view, projection) ;
        draw(vehicule2_, joueur2_.transform(), view, projection) ;

        /************* CONFIG SHADER *************/
        glUseProgram(m_program);
        // textures
        program_use_texture(m_program, "texture0", 0, textures[0], samplers[0]);
        program_use_texture(m_program, "texture1", 1, textures[1], samplers[1]);
        program_use_texture(m_program, "texture2", 2, textures[2], samplers[2]);
        // phares
        program_uniform(m_program, "spotP1Pos", joueur1_.getPosition());  
        program_uniform(m_program, "spotP2Pos", joueur2_.getPosition());
        program_uniform(m_program, "spotP1Dir", joueur1_.getDirection());
        program_uniform(m_program, "spotP2Dir", joueur2_.getDirection());

        /************* DESSIN TERRAIN *************/
        terrain_.draw(m_program, Identity(), view, projection);
        terrain_.drawUnderBox(m_program, Identity(), view, projection);

        /************* DESSIN OBJETS *************/
        objectsManager_.draw(m_program, view, projection);
        
    }

    Transform getLightSource(){
        Point source(100.0, 100.f, 100.f);
        Point target(100.f, 100.f, 0.f);
        Vector up = cross(Vector(source), Vector(1.f, 0.f, 0.f));
        return Lookat(source, target, up);
    }

    Transform ortho(float right, float left, float top, float bottom, float far, float near){
        return Transform(2.0/(right - left), 0.0, 0.0, -(right + left)/(right - left),
                                    0.0, 2.0/(top - bottom), 0.0, -(top + bottom)/(top - bottom),
                                    0.0, 0.0, -2.0/(far - near), -(far + near)/(far - near)); 
    }

    /************* DESSIN *************/
    int render( )
    {
        /* Mise à jour de la scene et du score */
        updateScene();
        if(score_.getRoundWinner() == -1) {
            updateScores();
        }

        /* Déplace la caméra & récupère la projection */
        Transform view = updateCamera(score_.getRoundWinner());
        /*try{
            updateCamera(score_.getRoundWinner());
        }
        catch ( const std::logic_error & e ) 
        { 
            std::cerr << e.what(); 
            return 1;
        }*/
        
        Transform projection = Perspective(90, (float) window_width() / (float) window_height(), 0.1f, 100.0f);

        /* Calcul de la shadowmap */
        glEnable(GL_CULL_FACE);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer_);
            glViewport(0, 0, shadowWidth_, shadowHeight_);
            glClear(GL_DEPTH_BUFFER_BIT);
            glCullFace(GL_FRONT);

            // Set du shader de génération de shadowmap
            glUseProgram(shadow_program);

            Clock::time_point time = Clock::now();
            float delta = 5 * (float)std::chrono::duration_cast<std::chrono::milliseconds>(time - start_).count() / 1000.0;
            while(delta > 360.0)
                delta -= 360.0;

            // Définition des transformations de la lumière
            Transform lightView = getLightSource();// * RotationY(delta);
            Transform lightProjection = ortho(200.0, -200.0, 200.0, -200.0, 400.0, 0.0);
            //Transform lightProjection = projection;

            // Rendu de la scene
            terrain_.draw(shadow_program, Identity(), lightView, lightProjection);
            terrain_.drawUnderBox(shadow_program, Translation(-100.0, -100.0, 0.0) * Scale(2.0, 2.0, 1.0), lightView, lightProjection);
            program_uniform(shadow_program, "mvpMatrix",
                            lightProjection * lightView * joueur1_.transform());
            vehicule1_.draw(shadow_program);
            program_uniform(shadow_program, "mvpMatrix", 
                            lightProjection * lightView * joueur2_.transform());  
            vehicule2_.draw(shadow_program);
        // Remise au valeurs par défaut de l'affichage
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // glDisable(GL_CULL_FACE);
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);

        /* Affichage de la scene et de l'interface */ 
        // Set du shader principal
        glUseProgram(m_program);

        // Passage de la shadowmap et de la transformation de la lumière
        program_use_texture(m_program, "shadowMap", 3, shadowMap_);
        program_uniform(m_program, "lightPos", lightView);
        program_uniform(m_program, "lightProj", lightProjection);

        // Rendu de la scene et de l'interface
        renderScene(view, projection);
        score_.draw();

        /* s'il y a un gagnant, affichage spécifique pendant 5 s */
        // time = Clock::now();
        // float winner_delay = (float)std::chrono::duration_cast<std::chrono::milliseconds>(time - winner_time_).count() / 1000.0;
        // if(score_.end()) {
        //     score_.drawWinner();
        //     if (winner_delay > 5.0) {
        //             return 0;
        //     }
        // }else {
        //     if(score_.getRoundWinner() != -1) {
        //         score_.drawRoundWinner();
        //         if (winner_delay > 5.0) {
        //             reset();
        //         }
        //     }
        // }

        

        /* Contrôles clavier */
        //reset
        if(key_state('r')) {
            joueur1_.spawn_at(Point(89.0,27.0,0), Vector(1,0,0)) ;
            joueur1_.activate() ;
            joueur2_.spawn_at(Point(89.0,25.0,0), Vector(1,0,0)) ;
            joueur2_.activate() ;

            oldPmin_ = Point(std::min(joueur1_.get_x(), joueur2_.get_x()),
                        std::min(joueur1_.get_y(), joueur2_.get_y()),
                        std::max(joueur1_.get_z(), joueur2_.get_z()));
            oldPmax_ = Point(std::max(joueur1_.get_x(), joueur2_.get_x()),
                        std::max(joueur1_.get_y(), joueur2_.get_y()),
                        std::max(joueur1_.get_z(), joueur2_.get_z()));
        }

        return 1;
    }

    void reset() {
        // reset joueurs
        joueur1_.spawn_at(Point(89.0,27.0,0), Vector(1,0,0)) ;
        joueur1_.activate() ;
        joueur2_.spawn_at(Point(89.0,25.0,0), Vector(1,0,0)) ;
        joueur2_.activate() ;

        // reset caméra
        oldPmin_ = Point(std::min(joueur1_.get_x(), joueur2_.get_x()),
                    std::min(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));
        oldPmax_ = Point(std::max(joueur1_.get_x(), joueur2_.get_x()),
                    std::max(joueur1_.get_y(), joueur2_.get_y()),
                    std::max(joueur1_.get_z(), joueur2_.get_z()));

        // reset score de la manche
        score_.resetRound();
    }

protected:
    Mesh vehicule1_;
    Mesh vehicule2_;
    Player joueur1_;
    Player joueur2_;
    KeyboardController controller1_ ;
    KeyboardController controller2_ ;

    GeneratedTerrain terrain_;
    std::vector<GLuint> textures;
    std::vector<GLuint> samplers;

    ObjectsManager objectsManager_;

    Point oldPmin_;
    Point oldPmax_;
    std::chrono::high_resolution_clock::time_point oldTime_;

    GLuint m_program;

    GLuint shadowBuffer_;
    GLuint shadowMap_;
    GLuint shadow_program;
    int shadowWidth_, shadowHeight_;

    unsigned int max_score_;
    unsigned int score_player1_;

    Clock::time_point start_;

    ScoreManager score_;
    std::chrono::high_resolution_clock::time_point winner_time_;
};


int main( int argc, char **argv )
{
    Play app;
    app.run();
    
    return 0;
}
