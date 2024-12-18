## Instruções de uso

### REQUISITOS
- Make
- [cJSON](https://github.com/DaveGamble/cJSON). Para instalar o cJSON tente ir em cJSON/build e rodar um comando make e em seguida make install.  Caso não funcione siga as instruções dos autores da biblioteca.

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
