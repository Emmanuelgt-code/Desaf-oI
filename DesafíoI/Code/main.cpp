/*
 * Programa demostrativo de manipulaciónprocesamiento de imágenes BMP en C++ usando Qt.
 *
 * Descripción:
 * Este programa realiza las siguientes tareas:
 * 1. Carga una imagen BMP desde un archivo (formato RGB sin usar estructuras ni STL).
 * 2. Modifica los valores RGB de los píxeles asignando un degradado artificial basado en su posición.
 * 3. Exporta la imagen modificada a un nuevo archivo BMP.
 * 4. Carga un archivo de texto que contiene una semilla (offset) y los resultados del enmascaramiento
 *    aplicados a una versión transformada de la imagen, en forma de tripletas RGB.
 * 5. Muestra en consola los valores cargados desde el archivo de enmascaramiento.
 * 6. Gestiona la memoria dinámicamente, liberando los recursos utilizados.
 *
 * Entradas:
 * - Archivo de imagen BMP de entrada ("I_O.bmp").
 * - Archivo de salida para guardar la imagen modificada ("I_D.bmp").
 * - Archivo de texto ("M1.txt") que contiene:
 *     • Una línea con la semilla inicial (offset).
 *     • Varias líneas con tripletas RGB resultantes del enmascaramiento.
 *
 * Salidas:
 * - Imagen BMP modificada ("I_D.bmp").
 * - Datos RGB leídos desde el archivo de enmascaramiento impresos por consola.
 *
 * Requiere:
 * - Librerías Qt para manejo de imágenes (QImage, QString).
 * - No utiliza estructuras ni STL. Solo arreglos dinámicos y memoria básica de C++.
 *
 * Autores: Augusto Salazar Y Aníbal Guerra
 * Fecha: 06/04/2025
 * Asistencia de ChatGPT para mejorar la forma y presentación del código fuente
 */



//AUTORES:

//Emmanuel Guerra Tuberquia
//Maria Valentina Quiroga Alzate

#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include <cstdio>  //Solo para sprintf

using namespace std;

unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);

int contarTransformaciones();
unsigned char* desplazarDerechaImagen(unsigned char* img, int dataSize, int bits);
unsigned char* desplazarIzquierdaImagen(unsigned char* img, int dataSize, int bits);
unsigned char* XOR(unsigned char* img1, unsigned char* img2, int dataSize);
unsigned char* rotarImagenIzquierda(unsigned char* img, int dataSize, int bits);
unsigned char* rotarImagenDerecha(unsigned char* img, int dataSize, int bits);
unsigned char* extraerBloqueIM(unsigned char* imgIM, int width, int height, int widthMascara, int heightMascara, int semilla);
unsigned char* extraerBloqueP(unsigned char* imgP, int width, int height, int widthMascara, int heightMascara, int semilla);
int calcularDiferencia(unsigned char* bloqueTransformado, unsigned int* datosEnmascarados, int n_pixels, bool ignorarDesplazamiento);

int identificarTransformacion(unsigned char* bloque, unsigned char* imgMascara, unsigned int* datosEnmascarados, int dataSize, int n_pixels, unsigned char* bloqueIM);
unsigned char* aplicarTransformacionInversa(unsigned char* imgP, int tipo, int dataSize, unsigned char* imgIM);



int main(){

    const char* nombresTransformaciones[33] = {"XOR",
        "Desplazamiento a la izquierda 1 bit", "Desplazamiento a la izquierda 2 bits", "Desplazamiento a la izquierda 3 bits", "Desplazamiento a la izquierda 4 bits", "Desplazamiento a la izquierda 5 bits", "Desplazamiento a la izquierda 6 bits", "Desplazamiento a la izquierda 7 bits", "Desplazamiento a la izquierda 8 bits",
        "Desplazamiento a la derecha 1 bit", "Desplazamiento a la derecha 2 bits", "Desplazamiento a la derecha 3 bits", "Desplazamiento a la derecha 4 bits", "Desplazamiento a la derecha 5 bits", "Desplazamiento a la derecha 6 bits", "Desplazamiento a la derecha 7 bits", "Desplazamiento a la derecha 8 bits",
        "Rotacion a la izquierda 1 bit", "Rotacion a la izquierda 2 bits", "Rotacion a la izquierda 3 bits", "Rotacion a la izquierda 4 bits", "Rotacion a la izquierda 5 bits", "Rotacion a la izquierda 6 bits", "Rotacion a la izquierda 7 bits", "Rotacion a la izquierda 8 bits",
        "Rotacion a la derecha 1 bit", "Rotacion a la derecha 2 bits", "Rotacion a la derecha 3 bits", "Rotacion a la derecha 4 bits", "Rotacion a la derecha 5 bits", "Rotacion a la derecha 6 bits", "Rotacion a la derecha 7 bits", "Rotacion a la derecha 8 bits"
    };


    int totalEtapas = contarTransformaciones();

    int width = 0;
    int height = 0;

    int widthMascara=0;
    int heightMascara=0;

    unsigned char* imgIM = loadPixels("I_M.bmp", width, height);
    if (!imgIM) {
        qDebug() << "Error al cargar I_M.bmp";
        return 0;
    }
    unsigned char* imgMascara = loadPixels("M.bmp", widthMascara, heightMascara);
    if (!imgMascara) {
        qDebug() << "Error al cargar M.bmp";
        return 0;
    }
    unsigned char* imgP = loadPixels("I_D.bmp", width, height);
    if (!imgP) {
        qDebug() << "Error al cargar I_D.bmp";
        return 0;
    }
    cout << "Tamano de I_M.bmp: " << width << " x " << height << endl;
    cout << "Tamano de M.bmp: " << widthMascara << " x " << heightMascara << endl;
    cout << "Tamano de I_D.bmp: " << width << " x " << height << endl;


    int totalPix = width * height * 3;

    //For para las etapas donde se lee el archivo, se llama a la funcion de combinaciones y se realiza el proceso inverso
    for (int etapa = totalEtapas - 1; etapa >= 0; etapa--) {
        //Esto genera "M0.txt", "M1.txt",... hasta "M10000.txt", si es necesario y si el espacio es pequeño no se va a perder
        char* archivoTxt = new char[20];
        sprintf(archivoTxt, "M%d.txt", etapa);
        //maskingData son los .txt
        int seed = 0;
        int n_pixels = 0;
        unsigned int *maskingData = loadSeedMasking(archivoTxt, seed, n_pixels);
        if (!maskingData) {
            qDebug() << "Error al cargar datos del enmascaramiento de " << archivoTxt;
            return 0;
        }
        //Para hacer pruebas
        /*for(int i=0;i<2;i++){
            cout<<"Masking data:";
            cout<<(int)maskingData[i]<<endl;
        };*/

        //Para confirma en que estapa esta y si está leyendo bien la semilla
        //qDebug() << "Etapa" << etapa << " Semilla:" << seed << " Pixeles:" << n_pixels;

        //La variable bloque es de P_n
        unsigned char* bloque = extraerBloqueP(imgP, width, height, widthMascara, heightMascara, seed);

        if (!bloque) {
            qDebug() << "Error al extraer el bloque de P";
            delete[] maskingData;
            return 0;
        }
        //Para hacer pruebas
        /*for(int i=0;i<2;i++){
            cout<<"P:";
            cout<<(int)bloque[i]<<endl;
        };*/

        unsigned char* bloqueIM = extraerBloqueIM(imgIM, width, height, widthMascara, heightMascara, seed);
        if (!bloqueIM) {
            qDebug() << "Error al extraer el bloque de I_M";
            delete[] maskingData;
            return 0;
        }
        //Para hacer pruebas
        /*for(int i=0;i<2;i++){
            cout<<"IM:";
            cout<<(int)bloqueIM[i]<<endl;
        };*/

        int tipoTransformacion = identificarTransformacion(bloque, imgMascara, maskingData, widthMascara * heightMascara * 3, n_pixels, bloqueIM);
        //Al identificar el tipo de transformación imprime la etapa y que transformacion fue la mejor
        if (tipoTransformacion >= 0 && tipoTransformacion < 33) {
            cout << "Etapa " << etapa << ": " << nombresTransformaciones[tipoTransformacion] << endl;
        } else {
            cout << "Etapa " << etapa << ": transformacion desconocida (indice " << tipoTransformacion << ")" << endl;
        }

        unsigned char* imgP_n_menos_1 = aplicarTransformacionInversa(imgP, tipoTransformacion, totalPix, imgIM);

        //Esta linea la dejamos comentada para evidenciar uno de lo errores que presentamos
        //ya que estabamos aplicando la transformacion inversa solo al bloque
        //unsigned char* imgP_n_menos_1 = aplicarTransformacionInversa(bloque, tipoTransformacion, width * height * 3, bloqueIM);

        //Se libera la imagen anterior
        delete[] imgP;

        //Se avanza a la siguiente etapa
        imgP = imgP_n_menos_1;

        char* PReconstruida= new char[20];
        sprintf(PReconstruida, "P%d.bmp", etapa);

        exportImage(imgP, width, height, PReconstruida);
        qDebug() << "Imagen guardada:" << PReconstruida;

        delete[] maskingData;
        delete[] bloque;
        delete[] archivoTxt;
        delete[] PReconstruida;


    }

    const char* archivoSalida = "I_O.bmp";
    exportImage(imgP, width, height, archivoSalida);
    qDebug() << "Llegamos al resultado final: " << archivoSalida;
    delete[] imgP;  // liberar memoria final

    return 0; // Fin del programa

}


unsigned char* loadPixels(QString input, int &width, int &height){
    /*
 * @brief Carga una imagen BMP desde un archivo y extrae los datos de píxeles en formato RGB.
 *
 * Esta función utiliza la clase QImage de Qt para abrir una imagen en formato BMP, convertirla al
 * formato RGB888 (24 bits: 8 bits por canal), y copiar sus datos de píxeles a un arreglo dinámico
 * de tipo unsigned char. El arreglo contendrá los valores de los canales Rojo, Verde y Azul (R, G, B)
 * de cada píxel de la imagen, sin rellenos (padding).
 *
 * @param input Ruta del archivo de imagen BMP a cargar (tipo QString).
 * @param width Parámetro de salida que contendrá el ancho de la imagen cargada (en píxeles).
 * @param height Parámetro de salida que contendrá la altura de la imagen cargada (en píxeles).
 * @return Puntero a un arreglo dinámico que contiene los datos de los píxeles en formato RGB.
 *         Devuelve nullptr si la imagen no pudo cargarse.
 *
 * @note Es responsabilidad del usuario liberar la memoria asignada al arreglo devuelto usando delete[].
 */

    // Cargar la imagen BMP desde el archivo especificado (usando Qt)
    QImage imagen(input);

    // Verifica si la imagen fue cargada correctamente
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << std::endl;
        return nullptr; // Retorna un puntero nulo si la carga falló
    }

    // Convierte la imagen al formato RGB888 (3 canales de 8 bits sin transparencia)
    imagen = imagen.convertToFormat(QImage::Format_RGB888);

    // Obtiene el ancho y el alto de la imagen cargada
    width = imagen.width();
    height = imagen.height();

    // Calcula el tamaño total de datos (3 bytes por píxel: R, G, B)
    int dataSize = width * height * 3;

    // Reserva memoria dinámica para almacenar los valores RGB de cada píxel
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copia cada línea de píxeles de la imagen Qt a nuestro arreglo lineal
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);              // Línea original de la imagen con posible padding
        unsigned char* dstLine = pixelData + y * width * 3;     // Línea destino en el arreglo lineal sin padding
        memcpy(dstLine, srcLine, width * 3);                    // Copia los píxeles RGB de esa línea (sin padding)
    }

    /*cout << "Primeros 104 bytes: ";
    for (int i = 0; i < 104; ++i)
        cout << (int)pixelData[i] << " ";
    cout << endl;*/

    // Retorna el puntero al arreglo de datos de píxeles cargado en memoria
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida){
    /*
 * @brief Exporta una imagen en formato BMP a partir de un arreglo de píxeles en formato RGB.
 *
 * Esta función crea una imagen de tipo QImage utilizando los datos contenidos en el arreglo dinámico
 * pixelData, que debe representar una imagen en formato RGB888 (3 bytes por píxel, sin padding).
 * A continuación, copia los datos línea por línea a la imagen de salida y guarda el archivo resultante
 * en formato BMP en la ruta especificada.
 *
 * @param pixelData Puntero a un arreglo de bytes que contiene los datos RGB de la imagen a exportar.
 *                  El tamaño debe ser igual a width * height * 3 bytes.
 * @param width Ancho de la imagen en píxeles.
 * @param height Alto de la imagen en píxeles.
 * @param archivoSalida Ruta y nombre del archivo de salida en el que se guardará la imagen BMP (QString).
 *
 * @return true si la imagen se guardó exitosamente; false si ocurrió un error durante el proceso.
 *
 * @note La función no libera la memoria del arreglo pixelData; esta responsabilidad recae en el usuario.
 */

    // Crear una nueva imagen de salida con el mismo tamaño que la original
    // usando el formato RGB888 (3 bytes por píxel, sin canal alfa)
    QImage outputImage(width, height, QImage::Format_RGB888);

    // Copiar los datos de píxeles desde el buffer al objeto QImage
    for (int y = 0; y < height; ++y) {
        // outputImage.scanLine(y) devuelve un puntero a la línea y-ésima de píxeles en la imagen
        // pixelData + y * width * 3 apunta al inicio de la línea y-ésima en el buffer (sin padding)
        // width * 3 son los bytes a copiar (3 por píxel)
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }

    // Guardar la imagen en disco como archivo BMP
    if (!outputImage.save(archivoSalida, "BMP")) {
        // Si hubo un error al guardar, mostrar mensaje de error
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false; // Indica que la operación falló
    } else {
        // Si la imagen fue guardada correctamente, mostrar mensaje de éxito
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true; // Indica éxito
    }

}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels){
    /*
 * @brief Carga la semilla y los resultados del enmascaramiento desde un archivo de texto.
 *
 * Esta función abre un archivo de texto que contiene una semilla en la primera línea y,
 * a continuación, una lista de valores RGB resultantes del proceso de enmascaramiento.
 * Primero cuenta cuántos tripletes de píxeles hay, luego reserva memoria dinámica
 * y finalmente carga los valores en un arreglo de enteros.
 *
 * @param nombreArchivo Ruta del archivo de texto que contiene la semilla y los valores RGB.
 * @param seed Variable de referencia donde se almacenará el valor entero de la semilla.
 * @param n_pixels Variable de referencia donde se almacenará la cantidad de píxeles leídos
 *                 (equivalente al número de líneas después de la semilla).
 *
 * @return Puntero a un arreglo dinámico de enteros que contiene los valores RGB
 *         en orden secuencial (R, G, B, R, G, B, ...). Devuelve nullptr si ocurre un error al abrir el archivo.
 *
 * @note Es responsabilidad del usuario liberar la memoria reservada con delete[].
 */

    // Abrir el archivo que contiene la semilla y los valores RGB
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        // Verificar si el archivo pudo abrirse correctamente
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }

    // Leer la semilla desde la primera línea del archivo
    archivo >> seed;

    int r, g, b;

    // Contar cuántos grupos de valores RGB hay en el archivo
    // Se asume que cada línea después de la semilla tiene tres valores (r, g, b)
    n_pixels = 0;
    while (archivo >> r >> g >> b) {
        n_pixels++;  // Contamos la cantidad de píxeles
    }

    // Cerrar el archivo para volver a abrirlo desde el inicio
    archivo.close();
    archivo.open(nombreArchivo);

    // Verificar que se pudo reabrir el archivo correctamente
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo." << endl;
        return nullptr;
    }

    // Reservar memoria dinámica para guardar todos los valores RGB
    // Cada píxel tiene 3 componentes: R, G y B
    unsigned int* RGB = new unsigned int[n_pixels * 3];

    // Leer nuevamente la semilla desde el archivo (se descarta su valor porque ya se cargó antes)
    archivo >> seed;

    // Leer y almacenar los valores RGB uno por uno en el arreglo dinámico
    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i] = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }

    // Cerrar el archivo después de terminar la lectura
    archivo.close();

    // Mostrar información de control en consola
    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de pixeles leidos: " << n_pixels << endl;

    // Retornar el puntero al arreglo con los datos RGB
    return RGB;
}


//Funciones construidas

int contarTransformaciones() {
    int num_transf;
    cout<<"Ingresa el numero de transformaciones que se realizaron (Cantidad de archivos .txt contando el 0): ";
    cin>> num_transf;

    return num_transf;
}



//Transformaciones


unsigned char* desplazarDerechaImagen(unsigned char* img, int dataSize, int bits) {
    unsigned char* resultado = new unsigned char[dataSize];
    bits %= 8;  //Asegura que bits no sea mayor que 8
    for (int i = 0; i < dataSize; i++) {
        resultado[i] = img[i] >> bits;  //Desplazamiento a la derecha
    }
    return resultado;
}

unsigned char* desplazarIzquierdaImagen(unsigned char* img, int dataSize, int bits) {
    unsigned char* resultado = new unsigned char[dataSize];
    bits %= 8;  //Asegura que bits no sea mayor que 8
    for (int i = 0; i < dataSize; i++) {
        resultado[i] = img[i] << bits;  //Desplazamiento a la izquierda
    }
    return resultado;
}


unsigned char* XOR(unsigned char* img1, unsigned char* img2, int dataSize) {
    unsigned char* resultado = new unsigned char[dataSize];
    for (int i = 0; i < dataSize; ++i)
        resultado[i] = img1[i] ^ img2[i];
    return resultado;
}

unsigned char* rotarImagenIzquierda(unsigned char* img, int dataSize, int bits) {
    unsigned char* resultado = new unsigned char[dataSize];
    bits %= 8;
    for (int i = 0; i < dataSize; i++)
        resultado[i] = (img[i] << bits) | (img[i] >> (8 - bits));
    return resultado;
}

unsigned char* rotarImagenDerecha(unsigned char* img, int dataSize, int bits) {
    unsigned char* resultado = new unsigned char[dataSize];
    bits %= 8;
    for (int i = 0; i < dataSize; i++)
        resultado[i] = (img[i] >> bits) | (img[i] << (8 - bits));
    return resultado;
}


//Extrae el pedacito de I_M del tamaño del .txt
unsigned char* extraerBloqueIM(unsigned char* imgIM, int width, int height, int widthMascara, int heightMascara, int seed) {
    //Tamaño de la máscara (número de bytes a extraer)
    int mascaraSize = widthMascara * heightMascara * 3; //Porque cada píxel tiene 3 componentes (RGB)

    //Se verifica si la semilla está dentro del rango
    if (seed >= width * height * 3 || seed < 0) {
        cout << "Error: Semilla fuera de rango." <<endl;
        return 0;
    }

    //Reservamos memoria para el bloque extraído I_M
    unsigned char* bloqueExtraidoIM = new unsigned char[mascaraSize];

    //Copiamos los píxeles desde imgIM en el rango determinado por la semilla y el tamaño de la máscara
    memcpy(bloqueExtraidoIM, imgIM + seed, mascaraSize);

    return bloqueExtraidoIM;
}

unsigned char* extraerBloqueP(unsigned char* imgP, int width, int height, int widthMascara, int heightMascara, int seed) {
    //Tamaño de la máscara (número de bytes a extraer)
    int mascaraSize = widthMascara * heightMascara * 3; //Porque cada píxel tiene 3 componentes (RGB)

    //Se verifica si la semilla está dentro del rango válido
    if (seed >= width * height * 3 || seed < 0) {
        cout << "Error: Semilla fuera de rango." << endl;
        return 0;
    }

    //Reservamos memoria para el bloque extraído
    unsigned char* bloqueExtraidoP = new unsigned char[mascaraSize];

    //Copiamos los píxeles desde imgP en el rango determinado por la semilla y el tamaño de la máscara
    memcpy(bloqueExtraidoP, imgP + seed, mascaraSize);

    return bloqueExtraidoP;
}


//Las siguientes funciones son para el analisis de las transformaciones
int calcularDiferencia(unsigned int* bloqueTransformado, unsigned int* datosEnmascarados, int n_pixels) {
    int diferencia = 0;

    //Iteramos sobre cada píxeles
    for (int i = 0; i < n_pixels * 3; ++i) {
        //Para verficar se imprime los valores de bloqueTransformado y datosEnmascarados para ver lo que estamos comparando
        //cout << "Píxel " << i << " bloqueTransformado: " << (int)bloqueTransformado[i] << " datosEnmascarados: " << (int)datosEnmascarados[i] << endl;

        diferencia += abs((int)bloqueTransformado[i] - (int)datosEnmascarados[i]);
    }

    return diferencia;
}



//Función que prueba cada combinación
int identificarTransformacion(unsigned char* bloque, unsigned char* imgMascara, unsigned int* datosEnmascarados, int dataSize, int n_pixels, unsigned char* bloqueIM) {
    int mejorIndice = -1;
    int menorDiferencia = 10000000; //Ponemos una menor diferencia muy alta para evitar problemas

    for (int tipo = 0; tipo < 33; ++tipo) {
        unsigned char* transformado = aplicarTransformacionInversa(bloque, tipo, dataSize, bloqueIM);

        //Arreglo auxiliar para guardar la suma porque con la mascara supera el limite del char
        unsigned int* sumaCompleta = new unsigned int[dataSize];

        for (int i = 0; i < dataSize; ++i) {
            int suma = (int)transformado[i] + (int)imgMascara[i];
            sumaCompleta[i] = suma;
        }

        int dif = calcularDiferencia(sumaCompleta, datosEnmascarados, n_pixels);

        if (dif < menorDiferencia) {
            menorDiferencia = dif;
            mejorIndice = tipo;
            if (dif == 0) {
                delete[] transformado;
                break;  //si encientra la transformación exacta encontrada se sale
            }
        }

        delete[] transformado;
    }

    return mejorIndice;  //devuelve el índice de la transformación inversa usada
}

//Se aplica la transformacion inversa a la imagen completa segun el mejor indice(Tipo)
unsigned char* aplicarTransformacionInversa(unsigned char* imgP, int tipo, int dataSize, unsigned char* imgIM) {


    if (tipo == 0)
        return XOR(imgP, imgIM, dataSize);
    else if (tipo >= 1 && tipo <= 8)
        return desplazarIzquierdaImagen(imgP, dataSize, tipo);
    else if (tipo >= 9 && tipo <= 16)
        return desplazarDerechaImagen(imgP, dataSize, tipo - 8);
    else if (tipo >= 17 && tipo <= 24)
        return rotarImagenIzquierda(imgP, dataSize, tipo - 16);
    else if (tipo >= 25 && tipo <= 32)
        return rotarImagenDerecha(imgP, dataSize, tipo - 24);
    return nullptr;
}
