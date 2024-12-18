## Instruções de uso

### REQUISITOS
- Make
- [cJSON](https://github.com/DaveGamble/cJSON).

### Compilação
Para compilar o projeto, você pode utilizar um dos métodos abaixo:

#### Usando Make
Basta executar o comando:
```
make
```
#### Usando GCC diretamente
Caso prefira compilar manualmente, execute o seguinte comando:
```
gcc ./cJSON/cJSON.c main.c -o ./out/main -lm
```
Ambos os métodos devem ser executados na raiz do repositório.

**OBS**: É necessário que a biblioteca **cJSON** esteja instalada na raiz deste repositório.

### Execução

Para executar, basta rodar o arquivo gerado, localizado em ./out/.

### Análise de Algoritmo

## BackTrack

O algoritmo de backtracking tenta todas as combinações possíveis de alocação de aplicações. Ele verifica recursivamente, para cada aplicação, se é possível alocá-la em um satélite e, caso positivo, realiza a alocação temporária. Após processar uma aplicação, ele avança para a próxima, explorando diferentes possibilidades. Quando uma combinação é válida, o algoritmo calcula o número máximo de aplicações alocadas. Ao retornar, desfaz a última alocação para explorar outras opções.

Função backtrack(index, allocated):
    Se todas as aplicações foram processadas:
        Retornar allocated

    melhor_alocação ← allocated

    Para cada satélite em satellites:
        Se satélite cobre aplicação e pode alocar:
            Alocar aplicação no satélite
            melhor_alocação ← Máximo(melhor_alocação, 
                                     backtrack(index + 1, allocated + 1))
            Desalocar aplicação do satélite

    Retornar melhor_alocação

# Complexidade

Vamos considerar n = número de aplicações e m = número de satélites. A única parte do algoritmo que irá influenciar na sua complexidade é o loop e as chamadas recursivas (o resto será considerado constante).
Vamos chamar a nossa função recursiva de T(n). Podemos observar que essa função recursiva será chamada m vezes (número de satélites) e as recursões avançam o índice de um em um, ou seja, T(n-1). Portanto temos algo como:

            T(n) = m * (c + T(n-1))

Resolvendo para m e n temos:

    T(n) = m * (c + m *(c + T(n-2)))
    T(n) = m * (c + m *(c + m *(c + T(n-3))))
    T(n) = m * (c + m *(c + m *(c + m *(c + T(n-4)))))
    ...

Para T(1) = O(1) = c. Portanto:

    T(n) = m*c +m²*c + m³*c + m⁴*c + ... + m^n*c
    T(n) = c(m + m² + m³ + m⁴ + ... + m^n)
    T(n) = c(m^(n+1) - m)/(m-1)

Para grandes valores, o termo dominante será m^(n+1). Assim, podemos concluir que a complexidade é O(m^n)

## Greedy

O algoritmo greedy_allocate aloca as aplicações de forma gulosa, escolhendo o melhor satélite disponível para cada aplicação com base em sua capacidade de recursos (CPU + memória). A ideia central é processar cada aplicação, verificar quais satélites podem alocá-la e selecionar aquele com mais recursos disponíveis.

Para cada aplicação em apps:
    melhor_satellite ← NULL
    max_resources ← -1

    Para cada satélite em satellites:
        Se satélite cobre aplicação e pode alocar:
            recursos ← cpu_capacity + memory_capacity
            Se recursos > max_resources:
                melhor_satellite ← satélite
                max_resources ← recursos

    Se melhor_satellite ≠ NULL:
        Alocar aplicação no melhor_satellite

# Complexidade

