#include "raylib.h"
#include <cmath>
#include <vector>

int main() {
    //configuración
    ChangeDirectory(GetApplicationDirectory());
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED | FLAG_MSAA_4X_HINT);
    SetTargetFPS(60);
    InitWindow(1280, 720, "Pong");
    InitAudioDevice();

    ToggleFullscreen();

    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreen();
    }

    //sounds
    Music music = LoadMusicStream("assets/sounds/pong.mp3");
    Sound paddle = LoadSound("assets/sounds/paddle.wav");
    Sound border = LoadSound("assets/sounds/border.wav");
    Sound powerUp = LoadSound("assets/sounds/powerUp.wav");
    Sound notPowerUp = LoadSound("assets/sounds/notPowerUp.wav");
    Sound goal = LoadSound("assets/sounds/goal.wav");
    Sound win = LoadSound("assets/sounds/win.mp3");
    Sound selectSound = LoadSound("assets/sounds/select.wav");
    SetSoundVolume(paddle, 0.08f);
    SetSoundVolume(border, 0.08f);
    SetSoundVolume(powerUp, 0.08f);
    SetSoundVolume(notPowerUp, 0.08f);
    SetSoundVolume(goal, 0.1f);
    SetSoundVolume(win, 0.5f);
    SetSoundVolume(selectSound, 0.25f);

    //texturas(imagenes)
    Texture2D textures_enhancer[6] = {
        LoadTexture("assets/img/thunderbolt.png"),
        LoadTexture("assets/img/ice.png"),
        LoadTexture("assets/img/shield.png"),
        LoadTexture("assets/img/controller_invert.png"),
        LoadTexture("assets/img/big.png"),
        LoadTexture("assets/img/low.png"),
    };
    Texture2D ballTex = LoadTexture("assets/img/ball.png");
    Texture2D blueWin = LoadTexture("assets/img/bluewin.png");
    Texture2D redWin = LoadTexture("assets/img/redwin.png");
    Texture2D title = LoadTexture("assets/img/title.png");


    PlayMusicStream(music);

    //goles
    int points1 = 0;
    int points2 = 0;

    //variable que controla de quien es el turno
    int turn = 0;

    //variable que controla el contador en pausa
    float pause = 4;

    //velocidad de la pelota
    float speed = 700;

    //velocidad de las paletas
    float paddle1_speed = 600;
    float paddle2_speed = 600;

    //alturas de las paletas
    float paddle1_height = 180;
    float paddle2_height = 180;

    //esto se utiliza para el despotenciador de invertir controles
    float paddle1_invert = false;
    float paddle2_invert = false;

    //controlador de tiempo de escudos
    float show_shield1 = 0;
    float show_shield2 = 0;

    //Vectores de figuras
    Rectangle paddle1 = {55, (float)GetScreenHeight() / 2 - paddle1_height / 2, 40, paddle1_height};
    Rectangle paddle2 = {(float)GetScreenWidth() -55 - 40, (float)GetScreenHeight() / 2 - paddle2_height / 2, 40, paddle2_height};

    Vector2 ball = {(float) GetScreenWidth() / 2, (float) GetScreenHeight() / 2};
    Vector2 directionBall = {
        GetRandomValue(0, 1) != 0 ? (float)1 : (float)-1,
        GetRandomValue(0, 1) != 0 ? (float)1 : (float)-1
    };

    Rectangle shield1 = {0, 0, 25, (float)GetScreenHeight()};
    Rectangle shield2 = {1255, 0, 25, (float)GetScreenHeight()};

    //potenciadores y despotenciadores
    std::vector<Vector2> speed_enhancer;
    std::vector<Vector2> speed_debuff;
    std::vector<Vector2> shield_enhancer;
    std::vector<Vector2> controller_invert;
    std::vector<Vector2> big_enhancer;
    std::vector<Vector2> big_debuff;

    std::vector<Vector2>* enhancers[6] = {
        &speed_enhancer,
        &speed_debuff,
        &shield_enhancer,
        &controller_invert,
        &big_enhancer,
        &big_debuff
    };

    //segundos de juego
    float seconds = 0;

    //controlar sonido cuadno termina el juego
    bool winPlayed = false;

    //controlar si mostrar el menú o no
    bool menu = true;

    //modo de juego
    int gamemode = 1;

    //tiempo de juego desde que se ejecutó
    float time = 0.0;

    while(!WindowShouldClose()) {
        time += 1 * GetFrameTime();

        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        UpdateMusicStream(music);

        //textos
        const char* counter = TextFormat("%d      %d", points1, points2);
        const char* paused = TextFormat("%d", (int)pause);
        const char* uno_vs_cpu = TextFormat("1  Jugador  ");
        const char* uno_vs_uno = TextFormat("2 Jugadores");

        if(menu) {
            if(IsKeyDown(KEY_UP)) gamemode = 1;
            if(IsKeyDown(KEY_DOWN)) gamemode = 2;
            if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) PlaySound(selectSound);
            if(IsKeyPressed(KEY_ENTER)) menu = false;
            if(CheckCollisionPointRec(GetMousePosition(), {(float)GetScreenWidth() / 2 - MeasureText(uno_vs_cpu, 30) / 2, (float)GetScreenHeight() / 2 - 30, (float)MeasureText(uno_vs_cpu, 30), 30}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gamemode = 1;
                PlaySound(selectSound);
                menu = false;
            }
            if(CheckCollisionPointRec(GetMousePosition(), {(float)GetScreenWidth() / 2 - MeasureText(uno_vs_uno, 30) / 2, (float)GetScreenHeight() / 2 + 30, (float)MeasureText(uno_vs_cpu, 30), 30}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gamemode = 2;
                PlaySound(selectSound);
                menu = false;
            }

        } else {
            if(pause > 0) {
                pause -= GetFrameTime();
            } else {
                if(points1 < 10 && points2 < 10) {
                    seconds += 1 * GetFrameTime();

                    //aumentar velocidad de la pelota en cada segundo
                    speed < 1300 ? speed += 20 * GetFrameTime() : speed = 1300;

                    //controles
                    if(IsKeyDown(KEY_W)) !paddle1_invert ? paddle1.y -= paddle1_speed * GetFrameTime() : paddle1.y += paddle1_speed * GetFrameTime();
                    if(IsKeyDown(KEY_S)) !paddle1_invert ? paddle1.y += paddle1_speed * GetFrameTime() : paddle1.y -= paddle1_speed * GetFrameTime();
                    if(gamemode == 1) {
                        float distance_y = ball.y - (paddle2.y + paddle2.height / 2);
                        if(distance_y > 30) paddle2.y += paddle2_speed * GetFrameTime();
                        if(distance_y <= -30) paddle2.y -= paddle2_speed * GetFrameTime();
                    } else {
                        if(IsKeyDown(KEY_UP)) !paddle2_invert ? paddle2.y -= paddle2_speed * GetFrameTime() : paddle2.y += paddle2_speed * GetFrameTime();
                        if(IsKeyDown(KEY_DOWN)) !paddle2_invert ? paddle2.y += paddle2_speed * GetFrameTime() : paddle2.y -= paddle2_speed * GetFrameTime();
                    }

                    //limitar movimiento de paletas al tocar borde
                    if(paddle1.y >= GetScreenHeight() - paddle1_height) paddle1.y = GetScreenHeight() - paddle1_height;
                    if(paddle1.y <= 0) paddle1.y = 0;
                    if(paddle2.y >= GetScreenHeight() - paddle2_height) paddle2.y = GetScreenHeight() - paddle2_height;
                    if(paddle2.y <= 0) paddle2.y = 0;

                    //movimiento de la pelota
                    ball.x += speed * directionBall.x * GetFrameTime();
                    ball.y += speed * directionBall.y * GetFrameTime();

                    //revotar la pelota al tocar borde
                    if(ball.y <= 0 + ballTex.width / 2) {
                        PlaySound(border);
                        directionBall.y = 1;
                    }
                    if(ball.y >= GetScreenHeight() - ballTex.width / 2) {
                        PlaySound(border);
                        directionBall.y = -1;
                    }

                    //colisiones de la pelota con las paletas o escudos
                    if(CheckCollisionCircleRec(ball, 25, paddle1) || (show_shield1 > 0 && CheckCollisionCircleRec(ball, 25, shield1))) {
                        PlaySound(paddle);
                        float factor;
                        if(CheckCollisionCircleRec(ball, 25, paddle1)) {
                            factor = paddle1_height > 0 ? 0.01f * (180.0f / paddle1_height) : 0.01f * (180.0f / 180);
                        } else {
                            factor = 0.0025f;
                        }
                        float difference = (paddle1.y + paddle1_height / 2) - ball.y;
                        directionBall.y = sin((difference * -1) * factor);
                        directionBall.x = sqrt(1 - directionBall.y * directionBall.y);
                        if (CheckCollisionCircleRec(ball, 25, shield1)) show_shield1 = 0;
                        turn = 1;
                    }
                    if(CheckCollisionCircleRec(ball, 25, paddle2) || (show_shield2 > 0 && CheckCollisionCircleRec(ball, 25, shield2))) {
                        PlaySound(paddle);
                            float factor;
                        if(CheckCollisionCircleRec(ball, 25, paddle2)) {
                            factor = paddle2_height > 0 ? 0.01f * (180.0f / paddle2_height) : 0.01f * (180.0f / 180);
                        } else {
                            factor = 0.0025f;
                        }
                        float diferencia = (paddle2.y + paddle2_height / 2) - ball.y;
                        directionBall.y = sin((diferencia * -1) * factor);
                        directionBall.x = sqrt(1 - directionBall.y * directionBall.y) * -1;
                        if (CheckCollisionCircleRec(ball, 25, shield2)) show_shield2 = 0;
                        turn = 2;
                    }

                    //al marcar un gol:
                    if(ball.x <= 0 + 25) {
                        PlaySound(goal);
                        ball.x = GetScreenWidth() / 2;
                        ball.y = GetScreenHeight() / 2;
                        pause = 4;
                        speed = 750;
                        for(int i = 0; i < 6; i++) {
                            enhancers[i]->clear();
                        }
                        //restablecer potenciadores
                        paddle1_speed = 600;
                        paddle1_height = 180;
                        paddle1.height = paddle1_height;
                        paddle1_invert = false;
                        show_shield1 = 0;
                        paddle2_speed = 600;
                        paddle2_height = 180;
                        paddle2.height = paddle2_height;
                        paddle2_invert = false;
                        show_shield2 = 0;
                        points2++;
                        turn = 0;
                        paddle1.y = GetScreenHeight() / 2 - paddle1_height / 2;
                        paddle2.y = GetScreenHeight() / 2 - paddle2_height / 2;
                    }
                    if(ball.x >= GetScreenWidth() - 25) {
                        PlaySound(goal);
                        ball.x = GetScreenWidth() / 2;
                        ball.y = GetScreenHeight() / 2;
                        pause = 4;
                        speed = 750;
                        for(int i = 0; i < 6; i++) {
                            enhancers[i]->clear();
                        }
                        //restablecer potenciadores
                        paddle1_speed = 600;
                        paddle1_height = 180;
                        paddle1.height = paddle1_height;
                        paddle1_invert = false;
                        show_shield1 = 0;
                        paddle2_speed = 600;
                        paddle2_height = 180;
                        paddle2.height = paddle2_height;
                        paddle2_invert = false;
                        show_shield2 = 0;
                        points1++;
                        turn = 0;
                        paddle1.y = GetScreenHeight() / 2 - paddle1_height / 2;
                        paddle2.y = GetScreenHeight() / 2 - paddle2_height / 2;
                    }

                    //bajarle el tiempo a los escudos activados en cada segundo
                    if(show_shield1 > 0) show_shield1 -= 1 * GetFrameTime();
                    if(show_shield2 > 0) show_shield2 -= 1 * GetFrameTime();

                    //mostrar potenciadores(o despotenciadores)
                    if(seconds >= 3.0f && turn > 0) {
                        //mostrar potenciadores(o despotenciadores)
                        int index_enhancer = GetRandomValue(0, 5);
                        Vector2 new_vector_enhancer = {
                            (float)GetRandomValue(150, GetScreenWidth() - 150),
                            (float)GetRandomValue(25, 695)
                        };
                        enhancers[index_enhancer]->push_back(new_vector_enhancer);

                        seconds = 0;
                    }

                    //colisiones de la pelota con los potenciadores
                    for(int i = 0; i < 6; i++) {
                        for(int j = enhancers[i]->size() - 1; j >= 0; j--) {
                            float size_texture = 0.25;
                            if(i == 3) size_texture = 0.125;
                            if(CheckCollisionCircles(ball, ballTex.width / 2, (*enhancers[i])[j], textures_enhancer[i].width * size_texture / 2)) {
                                if (i % 2 == 0) {
                                    PlaySound(powerUp);
                                } else {
                                    PlaySound(notPowerUp);
                                }
                                switch(i) {
                                    case 0: 
                                        if(turn == 1) {
                                            paddle1_speed += 300;
                                        } else if (turn == 2) {
                                            paddle2_speed += 300;
                                        }
                                        enhancers[i]->erase(enhancers[i]->begin() + j);
                                        break;
                                    case 1:
                                        if(turn == 1) {
                                            paddle1_speed -= 300;
                                        } else if (turn == 2) {
                                            paddle2_speed -= 300;
                                        }
                                        enhancers[i]->erase(enhancers[i]->begin() + j);
                                        break;
                                    case 2:
                                        if(turn == 1) {
                                            show_shield1 += 5;
                                        } else if (turn == 2) {
                                            show_shield2 += 5;
                                        }
                                        enhancers[i]->erase(enhancers[i]->begin() + j);
                                        break;
                                    case 3: 
                                        if(turn == 1) {
                                            paddle1_invert = !paddle1_invert;
                                        } else if (turn == 2) {
                                            paddle2_invert = !paddle2_invert;
                                        }
                                        enhancers[i]->erase(enhancers[i]->begin() + j);
                                        break;
                                    case 4:
                                        if(turn == 1) {
                                            paddle1_height += 90;
                                            paddle1.height = paddle1_height;
                                        } else if (turn == 2) {
                                            paddle2_height += 90;
                                            paddle2.height = paddle2_height;
                                        }
                                        enhancers[i]->erase(enhancers[i]->begin() + j);
                                        break;
                                    case 5:
                                        if(turn == 1) {
                                            paddle1_height -= 90;
                                            paddle1.height = paddle1_height;
                                        } else if (turn == 2) {
                                            paddle2_height -= 90;
                                            paddle2.height = paddle2_height;
                                        }
                                        enhancers[i]->erase(enhancers[i]->begin() + j);
                                        break;
                                }
                            } 
                        }
                    }
                }
        }
        }



        BeginDrawing();
            //limpiar pantalla
            ClearBackground(BLACK);

            if(menu) {
               DrawTexture(title, (float)GetScreenWidth() / 2 - title.width / 2, 50, WHITE);
               DrawText(uno_vs_cpu, (float)GetScreenWidth() / 2 - MeasureText(uno_vs_cpu, 30) / 2, GetScreenHeight() / 2 - 30, 30, gamemode == 1 ? WHITE : GRAY);
               DrawText(uno_vs_uno, (float)GetScreenWidth() / 2 - MeasureText(uno_vs_uno, 30) / 2, GetScreenHeight() / 2 + 30, 30, gamemode == 2 ? WHITE : GRAY);
            } else {
                if(points1 == 10 || points2 == 10) {
                    if(!winPlayed) {
                        PlaySound(win);
                        winPlayed = true;
                    }
                    if(points1 == 10) {
                        DrawTextureEx(blueWin, {(float)GetScreenWidth() / 2 - blueWin.width * 0.8f / 2, (float)GetScreenHeight() / 3 - blueWin.height * 0.8f / 2}, 0.0, 0.8, WHITE);
                    } else {
                        DrawTextureEx(redWin, {(float)GetScreenWidth() / 2 - redWin.width * 0.8f / 2, (float)GetScreenHeight() / 3 - redWin.height * 0.8f / 2}, 0.0, 0.8, WHITE);
                    }
                    DrawText("Presiona énter para volver al menú", GetScreenWidth() / 2 - MeasureText("Presiona Enter para volver al menú", 30) / 2, GetScreenHeight() - 180, 30, (int)time % 2 == 0 ? WHITE : GRAY);
                    if(IsKeyPressed(KEY_ENTER)) {
                        menu = true;
                        points1 = 0;
                        points2 = 0;
                        winPlayed = false;
                        pause = 4;
                        speed = 700;
                        ball.x = GetScreenWidth() / 2;
                        ball.y = GetScreenHeight() / 2;
                        directionBall = {
                            GetRandomValue(0, 1) != 0 ? (float)1 : (float)-1,
                            GetRandomValue(0, 1) != 0 ? (float)1 : (float)-1
                        };
                    }
                } else {
                    //linea punteada
                    for(int i = 0; i < GetScreenHeight(); i += 30) {
                        DrawRectangle(GetScreenWidth() / 2 - 2, i, 4, 15, WHITE);
                    }

                    //escudos
                    if(show_shield1 > 0) DrawRectangleRec(shield1, GREEN);
                    if(show_shield2 > 0) DrawRectangleRec(shield2, GREEN);

                    //marcador de goles
                    DrawText(counter, GetScreenWidth() / 2 - MeasureText(counter, 50) / 2, 55, 50, WHITE);
                    
                    //paletas
                    DrawRectangleRec(paddle1, RED);
                    DrawRectangleRec(paddle2, BLUE);

                    //si esta en pausa, contador, si no, pelota
                    if(pause > 0) {
                        DrawText(paused, GetScreenWidth() / 2 - MeasureText(paused, 50) / 2, GetScreenHeight() / 2 - 40, 80, WHITE);
                    } else {
                        //potenciadores o despotenciadores
                        for(int i = 0; i < 6; i++) {
                            for(int j = 0; j < (int)enhancers[i]->size(); j++) {
                                if(i == 3) {
                                    DrawTextureEx(textures_enhancer[i], (*enhancers[i])[j], 0.0f, 0.125f, WHITE);
                                } else {
                                    DrawTextureEx(textures_enhancer[i], (*enhancers[i])[j], 0.0f, 0.25f, WHITE);
                                }
                            }
                        }
                        DrawTexture(ballTex, ball.x - ballTex.width / 2, ball.y - ballTex.height / 2, WHITE);
                    }
                }
            }

            

            
            
        EndDrawing();
    }
    //liberar memoria ram, buena práctica
    for(int i = 0; i < 6; i++) {
        UnloadTexture(textures_enhancer[i]);
    }
    UnloadTexture(ballTex);
    UnloadTexture(blueWin);
    UnloadTexture(redWin);
    UnloadMusicStream(music);
    UnloadSound(paddle);
    UnloadSound(border);
    UnloadSound(powerUp);
    UnloadSound(notPowerUp);
    UnloadSound(goal);
    UnloadSound(win);
    UnloadSound(selectSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
