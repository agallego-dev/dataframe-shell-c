#ifndef LIB_H
#define LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

// Definicion de colores
#define BLACK         0
#define BLUE          1
#define GREEN         2
#define CYAN          3
#define RED           4
#define MAGENTA       5
#define BROWN         6
#define LIGHTGRAY     7
#define DARKGRAY      8
#define LIGHTBLUE     9
#define LIGHTGREEN   10
#define LIGHTCYAN    11
#define LIGHTRED     12
#define LIGHTMAGENTA 13
#define YELLOW       14
#define WHITE        15

// Prototipo de TextColor
void TextColor(int color);

typedef struct tm Fecha;

// Enumeracion de tipos de datos para columnas
typedef enum {
    TEXTO,
    NUMERICO,
    FECHA
} TipoDato;

// Estructura para una columna
typedef struct {
    char nombre[30];
    TipoDato tipo;
    void *datos;
    unsigned char *esNulo;
    int numFilas;
} Columna;

// Estructura para un dataframe
typedef struct {
    Columna *columnas;
    int numColumnas;
    int numFilas;
    int *indice;
    char nombre[51]; // Nombre del dataframe
} Dataframe;

// Alias para tipos FECHA: 'Fecha' alias de 'Fecha' (#include <time.h>)
typedef Fecha Fecha;

// Nodo para lista enlazada de dataframes
typedef struct NodoLista {
    Dataframe *df;
    struct NodoLista *siguiente;
} Nodo;

// Lista de dataframes
typedef struct {
    int numDFs;
    Nodo *primero;
} Lista;

// Prototipos de funciones principales
Dataframe* Load(const char *nomFichero, char sep, int cab);
void viewDataframe(const Dataframe *df, int n);
void freeDataframe(Dataframe *df);
void metaDataframe(const Dataframe *df);
void filterDataframe(Dataframe *df, const char *colName, const char *op, const char *value);
void saveDataframe(const Dataframe *df, const char *filename);
void delnull(Dataframe *df, const char *colName);
void delcolum(Dataframe *df, const char *colName);
void sortDataframe(Dataframe *df, const char *colName, const char *order);
void quarter(Dataframe *df, const char *colName, const char *newColName);
int renombrarDataframe(Dataframe *df, const char *nuevoNombre, Lista *lista);
void listarDataframes(const Lista *lista);
void prefixColumna(Dataframe *df, const char *columna, int n, const char *nuevaColumna);



#endif // LIB_H

