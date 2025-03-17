#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 480
#define DEBUG_PANEL_WIDTH 200
#define MAX_FORKS 7
#define MAX_DEBUG_MESSAGES 50
#define DEBUG_LINE_HEIGHT 20

#define DEBUG_MESSAGES_AREA_HEIGHT (WINDOW_HEIGHT/2 - 10)
#define PSTREE_AREA_START (WINDOW_HEIGHT/2 + 10)

typedef struct {
    SDL_Rect rect;
    double angle;
    pid_t pid;
} Fork;

char debugMessages[MAX_DEBUG_MESSAGES][256];
int debugMessageCount = 0;

void add_debug_message(const char *msg) {
    if (debugMessageCount >= MAX_DEBUG_MESSAGES) {
        for (int i = 1; i < MAX_DEBUG_MESSAGES; i++) {
            strcpy(debugMessages[i - 1], debugMessages[i]);
        }
        debugMessageCount = MAX_DEBUG_MESSAGES - 1;
    }
    strncpy(debugMessages[debugMessageCount], msg, 255);
    debugMessages[debugMessageCount][255] = '\0';
    debugMessageCount++;
}

void render_debug_text(SDL_Renderer *renderer, TTF_Font *font, const char *text, int y) {
    SDL_Color orange = {255, 165, 0, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, orange);
    if (!surface) return;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstRect = {10, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
}

void render_pstree(SDL_Renderer *renderer, TTF_Font *font, Fork forks[], int forkCount, int parentPID, int startY) {
    char line[256];
    int y = startY;
    snprintf(line, sizeof(line), "pstree:");
    render_debug_text(renderer, font, line, y);
    y += DEBUG_LINE_HEIGHT;
    snprintf(line, sizeof(line), "%d", parentPID);
    render_debug_text(renderer, font, line, y);
    y += DEBUG_LINE_HEIGHT;
    for (int i = 0; i < forkCount; i++) {
        if (y + DEBUG_LINE_HEIGHT > WINDOW_HEIGHT)
            break;
        snprintf(line, sizeof(line), (i < forkCount - 1) ? "|---PID:%d" : "|___PID:%d", forks[i].pid);
        render_debug_text(renderer, font, line, y);
        y += DEBUG_LINE_HEIGHT;
    }
}

int main(int argc, char *argv[]) {
    // Launch an external xterm to run pstree.
    if (fork() == 0) {
        execl("/usr/bin/xterm", "xterm", "-hold", "-e", "watch", "-n", "1", "pstree", "-p", NULL);
        exit(1);
    }
    
    // Initialize SDL systems.
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ||
        !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ||
        TTF_Init() == -1) {
        fprintf(stderr, "SDL/IMG/TTF Initialization Error\n");
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow("Tahaa's ForkPlate",
                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_Texture *bgTexture = IMG_LoadTexture(renderer, "plate_img.png");
    SDL_Texture *forkTexture = IMG_LoadTexture(renderer, "fork_img.png");
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    TTF_Font *smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 12);
    
    SDL_Rect button = {
        .w = 150,
        .h = 60,
        .x = DEBUG_PANEL_WIDTH + ((WINDOW_WIDTH - DEBUG_PANEL_WIDTH) - 150) / 2,
        .y = (WINDOW_HEIGHT - 60) / 2
    };
    
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, "FORK", white);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    int textW, textH;
    SDL_QueryTexture(textTexture, NULL, NULL, &textW, &textH);
    SDL_Rect textRect = {
        .w = textW, .h = textH,
        .x = button.x + (button.w - textW) / 2,
        .y = button.y + (button.h - textH) / 2
    };
    
    Fork forks[MAX_FORKS];
    int forkCount = 0, forkTexW, forkTexH;
    SDL_QueryTexture(forkTexture, NULL, NULL, &forkTexW, &forkTexH);
    pid_t parentPID = getpid();
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "Parent Plate PID: %d", parentPID);
        add_debug_message(buf);
    }
    
    int quit = 0;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) quit = 1;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (mx >= button.x && mx <= button.x + button.w &&
                    my >= button.y && my <= button.y + button.h && forkCount < MAX_FORKS) {
                    pid_t childPID = fork();
                    if (childPID == 0) {
                        prctl(PR_SET_NAME, "FORK", 0, 0, 0);
                        while (1) sleep(100);
                        exit(0);
                    } else if (childPID > 0) {
                        forks[forkCount].angle = rand() % 360;
                        forks[forkCount].rect.w = forkTexW;
                        forks[forkCount].rect.h = forkTexH;
                        forks[forkCount].rect.x = DEBUG_PANEL_WIDTH + (rand() % (WINDOW_WIDTH - DEBUG_PANEL_WIDTH - forkTexW));
                        forks[forkCount].rect.y = rand() % (WINDOW_HEIGHT - forkTexH);
                        forks[forkCount].pid = childPID;
                        char msg[256];
                        snprintf(msg, sizeof(msg), "Child Fork PID: %d", childPID);
                        add_debug_message(msg);
                        forkCount++;
                    }
                }
            }
        }
      
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
      
        SDL_Rect mainArea = {DEBUG_PANEL_WIDTH, 0, WINDOW_WIDTH - DEBUG_PANEL_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, bgTexture, NULL, &mainArea);
      
        SDL_Rect debugPanel = {0, 0, DEBUG_PANEL_WIDTH, WINDOW_HEIGHT};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &debugPanel);
      
        int y = 10;
        for (int i = 0; i < debugMessageCount; i++) {
            if (y + DEBUG_LINE_HEIGHT > WINDOW_HEIGHT/2)
                break;
            render_debug_text(renderer, font, debugMessages[i], y);
            y += DEBUG_LINE_HEIGHT;
        }
      
        render_pstree(renderer, font, forks, forkCount, parentPID, PSTREE_AREA_START);
      
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(renderer, &button);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &button);
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
      
        for (int i = 0; i < forkCount; i++) {
            SDL_RenderCopyEx(renderer, forkTexture, NULL, &forks[i].rect, forks[i].angle, NULL, SDL_FLIP_NONE);
            char pidText[32];
            snprintf(pidText, sizeof(pidText), "%d", forks[i].pid);
            SDL_Color whiteSmall = {255, 255, 255, 255};
            SDL_Surface *pidSurface = TTF_RenderText_Solid(smallFont, pidText, whiteSmall);
            if (pidSurface) {
                SDL_Texture *pidTexture = SDL_CreateTextureFromSurface(renderer, pidSurface);
                SDL_FreeSurface(pidSurface);
                SDL_Rect pidRect;
                SDL_QueryTexture(pidTexture, NULL, NULL, &pidRect.w, &pidRect.h);
                pidRect.x = forks[i].rect.x + forks[i].rect.w - pidRect.w - 2;
                pidRect.y = forks[i].rect.y + forks[i].rect.h - pidRect.h - 2;
                SDL_RenderCopy(renderer, pidTexture, NULL, &pidRect);
                SDL_DestroyTexture(pidTexture);
            }
        }
      
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
  
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(smallFont);
    TTF_CloseFont(font);
    SDL_DestroyTexture(forkTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}