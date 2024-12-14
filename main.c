// https://github.com/DaveGamble/cJSON
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cjson/cJSON.h>

#define MAX_SATELLITES 10
#define MAX_APPS 10

// Estrutura para representar um satélite
typedef struct {
    int id;
    int cpu_capacity;
    int memory_capacity;
    double position[3];  // Posição do satélite (x, y, z)
    double coverage_radius;
    int allocated_apps[MAX_APPS];  // Armazena os IDs das aplicações alocadas
    int allocated_count;  // Número de aplicações alocadas
} Satellite;
// strutura mais facil de percorrer
typedef struct {
    Satellite **satellites;
    int numero_satelites;
} ListaSatelites;

// Estrutura para representar uma aplicação
typedef struct {
    int id;
    int cpu_demand;
    int memory_demand;
    double position[3];  // Posição da aplicação
} Application;

// Função para calcular a distância Euclidiana entre o satélite e a aplicação
double calculate_distance(double sat_position[3], double app_position[3]) {
    return sqrt(pow(sat_position[0] - app_position[0], 2) + 
                pow(sat_position[1] - app_position[1], 2) + 
                pow(sat_position[2] - app_position[2], 2));
}

// Função para verificar se a aplicação está dentro da cobertura do satélite
int is_within_coverage(Satellite* sat, Application* app) {
    return calculate_distance(sat->position, app->position) <= sat->coverage_radius;
}

// Função para verificar se o satélite tem recursos suficientes para alocar a aplicação
int can_allocate(Satellite* sat, Application* app) {
    return sat->cpu_capacity >= app->cpu_demand && sat->memory_capacity >= app->memory_demand;
}

// Função para alocar a aplicação no satélite
void allocate(Satellite* sat, Application* app) {
    sat->cpu_capacity -= app->cpu_demand;
    sat->memory_capacity -= app->memory_demand;
    sat->allocated_apps[sat->allocated_count++] = app->id;
}

// Função para desalocar a aplicação do satélite
void deallocate(Satellite* sat, Application* app) {
    sat->cpu_capacity += app->cpu_demand;
    sat->memory_capacity += app->memory_demand;
    sat->allocated_count--;
}

// Função de backtracking para alocar as aplicações
int backtrack(Satellite satellites[], int num_satellites, Application apps[], int num_apps, int index, int allocated) {
    if (index == num_apps) {
        return allocated;  // Todas as aplicações foram alocadas
    }

    Application* app = &apps[index];
    int max_allocated = allocated;

    for (int i = 0; i < num_satellites; i++) {
        Satellite* sat = &satellites[i];

        if (is_within_coverage(sat, app) && can_allocate(sat, app)) {
            allocate(sat, app);
            max_allocated = fmax(max_allocated, backtrack(satellites, num_satellites, apps, num_apps, index + 1, allocated + 1));
            deallocate(sat, app);
        }
    }

    return max_allocated;
}

cJSON *le_json(char *nome_arquivo) {
    FILE *arquivo;
    cJSON *json;
    char *conteudo_arquivo;
    if((arquivo = fopen(nome_arquivo, "rb")) == NULL) {
        printf("\033[41mERRO:\033[0m Não foi possivel ler o arquivo %s", nome_arquivo);
        exit(1);
    }

    fseek(arquivo, 0, SEEK_END);
    long tam_arquivo = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    conteudo_arquivo = (char*)malloc(sizeof(char) * (tam_arquivo+1));

    fread(conteudo_arquivo, 1, tam_arquivo, arquivo);
    
    json = cJSON_Parse(conteudo_arquivo);
    if(json == NULL) {
        printf("\033[41mERRO:\033[0m Não foi possivel ler o json\n");
        goto end;
    }
    fclose(arquivo);
    return json;
end:
    fclose(arquivo);
    cJSON_Delete(json);
    exit(1);
    return ;
}
        


int main() {
    cJSON *json;
    FILE *arquivo = NULL;

    Satellite satellites[MAX_SATELLITES] = {
        {1, 100, 200, {0, 0, 0}, 50, {0}, 0},
        {2, 150, 250, {10, 10, 10}, 60, {0}, 0}
    };

    Application apps[MAX_APPS] = {
        {1, 50, 100, {5, 5, 5}},
        {2, 80, 150, {20, 20, 20}}
    };

    int num_satellites = 2;
    int num_apps = 2;

    
    json = le_json("./inputs/log1.json");
    printf("%s", cJSON_Print(json));
    int result = backtrack(satellites, num_satellites, apps, num_apps, 0, 0);
    // printf("Máximo de aplicações alocadas: %d\n", result);

    return 0;
}
