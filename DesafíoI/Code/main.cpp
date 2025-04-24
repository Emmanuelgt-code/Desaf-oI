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

#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include <QString>
#include <QDir>

#include <cstring> // Para memcpy
#include <bitset>
#define DEBUG
using std::bitset;


using namespace std;
unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width,int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);
int contarTransformaciones(QString path);
unsigned int* leerArchivoTXT(int etapa, int* seed, int* nPix);
unsigned char* XOR(unsigned char* img1, unsigned char* img2, int dataSize);
void checkImageDimensions(const QString& imagePath, unsigned int& width, unsigned int& height);
unsigned char* extraerBloqueIM(unsigned char* imgIM, int width, int height, int block_width, int block_height, int semilla);
unsigned char* extraerBloqueP(unsigned char* imgP, int width, int height, int block_width, int block_height, int semilla);
int calcularDiferencia(unsigned char* bloqueTransformado, unsigned int* datosEnmascarados, int n_pixels, bool ignorarDesplazamiento);

int identificarTransformacion(unsigned char* bloque, unsigned char* imgMascara, unsigned int* datosEnmascarados, int dataSize, int n_pixels, unsigned char* bloqueIM);
unsigned char* aplicarTransformacionInversa(unsigned char* imgP, int tipo, int dataSize, unsigned char* imgIM);



const char* nombresTransformaciones[33] = {
    "XOR", "Desplazamiento a la izquierda 1 bit", "Desplazamiento a la izquierda 2 bits", "Desplazamiento a la izquierda 3 bits",
    "Desplazamiento a la izquierda 4 bits", "Desplazamiento a la izquierda 5 bits", "Desplazamiento a la izquierda 6 bits",
    "Desplazamiento a la izquierda 7 bits", "Desplazamiento a la izquierda 8 bits",

    "Desplazamiento a la derecha 1 bit", "Desplazamiento a la derecha 2 bits", "Desplazamiento a la derecha 3 bits",
    "Desplazamiento a la derecha 4 bits", "Desplazamiento a la derecha 5 bits", "Desplazamiento a la derecha 6 bits",
    "Desplazamiento a la derecha 7 bits", "Desplazamiento a la derecha 8 bits",

    "Rotación a la izquierda 1 bit", "Rotación a la izquierda 2 bits", "Rotación a la izquierda 3 bits",
    "Rotación a la izquierda 4 bits", "Rotación a la izquierda 5 bits", "Rotación a la izquierda 6 bits",
    "Rotación a la izquierda 7 bits", "Rotación a la izquierda 8 bits",

    "Rotación a la derecha 1 bit", "Rotación a la derecha 2 bits", "Rotación a la derecha 3 bits",
    "Rotación a la derecha 4 bits", "Rotación a la derecha 5 bits", "Rotación a la derecha 6 bits",
    "Rotación a la derecha 7 bits", "Rotación a la derecha 8 bits"
};


int main(){

    QDir dir = QDir::current();
    qDebug() << "Directorio de ejecución:" << dir.absolutePath();

    QString ruta = ".";
    int totalEtapas = contarTransformaciones(ruta);

    int width = 0;
    int height = 0;

    int widthMascara=0, heightMascara=0;

    unsigned char* imgIM = loadPixels("I_M.bmp", width, height);
    unsigned char* imgMascara = loadPixels("M.bmp", widthMascara, heightMascara);
    for(int i=0;i<2;i++){
        cout<<"MASCARA:";
        cout<<(int)imgMascara[i]<<endl;
    };
    cout << "Tamaño de I_M.bmp: " << width << " x " << height << endl;
    cout << "Tamaño de M.bmp: " << widthMascara << " x " << heightMascara << endl;



    int totalPix = width * height *3;

    /*QString actualFile = QString("P%1.bmp").arg(totalEtapas-1);

    unsigned char* imgP = loadPixels(actualFile, width, height);

    if (!imgP) {
        qDebug() << "Error al cargar la P " << actualFile;
        return 1;
    }*/

    unsigned char* imgP = loadPixels("I_D.bmp", width, height);
    if (!imgP) {
        qDebug() << "Error al cargar I_D.bmp";
        return 1;
    }

    unsigned char* pixelData = nullptr;  // Aquí se guarda la imagen modificada después de las transformaciones


    //Como la idea es conocer el tamaño de la mascara para sacar un pedazo de P igual


    unsigned int block_width = 0;
    unsigned int block_height = 0;
    checkImageDimensions("M.bmp", block_width, block_height);

    //ESTA PARTE QUE SIGUE AUN NO FUNCIONA ERA UN EJEMPLO, PERO ESTA ES LA PARTE DONDE SE HACEN LAS COMBINACIONES y se extrae el bloque
    for (int etapa = totalEtapas - 1; etapa >= 0; etapa--) {

        QString archivoTxt = QString("M%1.txt").arg(etapa);

        int seed = 0;
        int n_pixels = 0;

        // maskingData son los .txt
        unsigned int *maskingData = loadSeedMasking(archivoTxt.toStdString().c_str(), seed, n_pixels);
        if (!maskingData) {
            qDebug() << "Error al cargar datos de enmascaramiento de " << archivoTxt;
            continue;
        }
        for(int i=0;i<2;i++){
            cout<<"Masking data:";
            cout<<(int)maskingData[i]<<endl;
        };

        qDebug() << "Etapa" << etapa << "- Semilla:" << seed << "- Pixeles:" << n_pixels;

        //bloque es de P_n
        unsigned char* bloque = extraerBloqueP(imgP, width, height, block_width, block_height, seed);
        if (!bloque) {
            delete[] maskingData;
            continue;
        }
        for(int i=0;i<2;i++){
            cout<<"P:";
            cout<<(int)bloque[i]<<endl;
        };



        unsigned char* bloqueIM = extraerBloqueIM(imgIM, width, height, block_width, block_height, seed);
        for(int i=0;i<2;i++){
            cout<<"IM:";
            cout<<(int)bloqueIM[i]<<endl;
        };

        int tipoTransformacion = identificarTransformacion(bloque, imgMascara, maskingData, block_width * block_height * 3, n_pixels, bloqueIM);

        //qDebug() << "Transformación identificada (índice):" << tipoTransformacion;
        // Al identificar el tipo de transformación

        if (tipoTransformacion >= 0 && tipoTransformacion < 33) {
            cout << "Etapa " << etapa << ": " << nombresTransformaciones[tipoTransformacion] << endl;
        } else {
            cout << "Etapa " << etapa << ": transformación desconocida (índice " << tipoTransformacion << ")" << endl;
        }
        //nt dataSize_block = block_width * block_height * 3;
        unsigned char* imgP_n_menos_1 = aplicarTransformacionInversa(imgP, tipoTransformacion, totalPix, imgIM);

        //unsigned char* imgP_n_menos_1 = aplicarTransformacionInversa(bloque, tipoTransformacion, width * height * 3, bloqueIM);


        // Liberar la imagen anterior
        delete[] imgP;

        // Avanzar a la nueva etapa
        imgP = imgP_n_menos_1;

        QString nombreEtapa = QString("P_reconstruida_%1.bmp").arg(etapa);
        exportImage(imgP, width, height, nombreEtapa);
        qDebug() << "Imagen guardada:" << nombreEtapa;


        //delete[] bloqueMascara;

        ///////////AQUI VOYY    YA SE DEBE CONTINUAR CON LAS COMBINACIONES
        /*XOR se cuenta como 1 combinación (usando una constante fija, por ejemplo 0xAA),
         *  y cada una de las operaciones SHL, SHR, ROL y ROR tiene 8 combinaciones posibles (
         *  una por cada desplazamiento de 1 a 8 bits), lo que da un total de 1 + 4×8 = 33 combinaciones.
         * /

        // Aplica la operación deseada (ej: XOR con bloque)
        /*pixelData = XOR(imgIM, bloque, width * height * 3);

        // Mostrar primeros valores
        for (int i = 0; i < std::min(5, n_pixels); ++i) {
            cout << "Pixel " << i << ": ("
                 << maskingData[i * 3] << ", "
                 << maskingData[i * 3 + 1] << ", "
                 << maskingData[i * 3 + 2] << ")" << endl;
        }*/

        delete[] maskingData;
        delete[] bloque;
    }

    QString archivoSalida = "I_O_RECUPERADA.bmp";
    bool exportI = exportImage(imgP, width, height, archivoSalida);
    cout << "Exportación final:" << exportI << endl;
    delete[] imgP;  // liberar memoria final


    // Definir archivo de salida
    /*QString archivoSalida = "I_O_FINAL.bmp"; // Aquí defines el nombre de archivo de salida


    // Exporta la imagen modificada a un nuevo archivo BMP
    bool exportI = exportImage(pixelData, width, height, archivoSalida);

    // Muestra si la exportación fue exitosa (true o false)
    cout << exportI << endl;
    */
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

    cout << "Dimensiones leídas: " << width << " x " << height << endl;
    cout << "Bytes esperados: " << width * height * 3 << endl;
    cout << "Primeros 104 bytes: ";
    for (int i = 0; i < 104; ++i)
        cout << (int)pixelData[i] << " ";
    cout << endl;


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
    cout << "Cantidad de píxeles leídos: " << n_pixels << endl;

    // Retornar el puntero al arreglo con los datos RGB
    return RGB;
}


/// CAMBIOS

int contarTransformaciones(QString path) {
    QDir dir(path);
    QStringList archivos = dir.entryList(QStringList() << "M*.txt", QDir::Files);
    qDebug() << "Archivos .txt encontrados:" << archivos;

    int maxIndice = -1;
    for (const QString& archivo : archivos) {
        QString nombre = archivo;
        nombre.remove("M").remove(".txt");  // adaptado a archivos tipo M0.txt
        bool ok;
        int indice = nombre.toInt(&ok);
        if (ok && indice > maxIndice) maxIndice = indice;
    }
    return maxIndice + 1;
}

//Transformaciones


unsigned char* desplazarDerechaImagen(unsigned char* img, int dataSize, int bits) {
    unsigned char* resultado = new unsigned char[dataSize];
    bits %= 8;  // Asegura que bits no sea mayor que 8
    for (int i = 0; i < dataSize; i++) {
        resultado[i] = img[i] >> bits;  // Desplazamiento a la derecha
    }
    return resultado;
}

// el total Pixeles debe ser columnas * filas
unsigned char* desplazarIzquierdaImagen(unsigned char* img, int dataSize, int bits) {
    unsigned char* resultado = new unsigned char[dataSize];
    bits %= 8;  // Asegura que bits no sea mayor que 8
    for (int i = 0; i < dataSize; i++) {
        resultado[i] = img[i] << bits;  // Desplazamiento a la izquierda
    }
    return resultado;
}


unsigned char* XOR(unsigned char* img1, unsigned char* img2, int dataSize) {
    unsigned char* resultado = new unsigned char[dataSize];
    cout << "Verificación exacta:\n";
    cout << "P[0]: " << (int)img1[0] << " (bin: " << bitset<8>(img1[0]) << ")\n";
    cout << "IM[0]: " << (int)img2[0] << " (bin: " << bitset<8>(img2[0]) << ")\n";
    cout << "XOR: " << (int)(img1[0] ^ img2[0]) << " (bin: " << bitset<8>(img1[0] ^ img2[0]) << ")\n";

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

unsigned int* leerArchivoTXT(int etapa, int* seed, int* nPix) {
    char nombreArchivo[20];
    sprintf(nombreArchivo, "M%d.txt", etapa);
    return loadSeedMasking(nombreArchivo, *seed, *nPix);
}

//Este sirve para mirar el tamaño de la mascara
void checkImageDimensions(const QString& imagePath, unsigned int& width, unsigned int& height) {
    QImage image(imagePath);
    if (image.isNull()) {
        qDebug() << "La imagen no se pudo cargar:" << imagePath;
    } else {
        width = image.width();
        height = image.height();
        qDebug() << "Dimensiones de" << imagePath << ":" << width << "x" << height;
    }
}

//Extrae el pedacito de p del tamaño del .txt
unsigned char* extraerBloqueIM(unsigned char* imgIM, int width, int height, int widthMascara, int heightMascara, int seed) {
    // Tamaño de la máscara (número de bytes a extraer)
    int mascaraSize = widthMascara * heightMascara * 3; // Cada píxel tiene 3 componentes (RGB)

    // Calculamos el inicio en la imagen imgIM
    int startIndex = seed; // Cada píxel tiene 3 valores (RGB), por lo que multiplicamos por 3 para obtener la posición correcta.

    // Verificamos si la semilla está dentro del rango válido
    if (startIndex >= width * height * 3 || startIndex < 0) {
        std::cerr << "Error: Semilla fuera de rango." << std::endl;
        return nullptr;
    }

    // Reservamos memoria para el bloque extraído
    unsigned char* bloqueExtraidoIM = new unsigned char[mascaraSize];

    // Copiamos los píxeles desde imgIM en el rango determinado por la semilla y el tamaño de la máscara
    memcpy(bloqueExtraidoIM, imgIM + startIndex, mascaraSize);

    return bloqueExtraidoIM;
}

unsigned char* extraerBloqueP(unsigned char* imgP, int width, int height, int widthMascara, int heightMascara, int seed) {
    // Tamaño de la máscara (número de bytes a extraer)
    int mascaraSize = widthMascara * heightMascara * 3; // Cada píxel tiene 3 componentes (RGB)

    // Calculamos el inicio en la imagen imgIM
    int startIndex = seed; // Cada píxel tiene 3 valores (RGB), por lo que multiplicamos por 3 para obtener la posición correcta.

    // Verificamos si la semilla está dentro del rango válido
    if (startIndex >= width * height * 3 || startIndex < 0) {
        std::cerr << "Error: Semilla fuera de rango." << std::endl;
        return nullptr;
    }

    // Reservamos memoria para el bloque extraído
    unsigned char* bloqueExtraidoP = new unsigned char[mascaraSize];

    // Copiamos los píxeles desde imgIM en el rango determinado por la semilla y el tamaño de la máscara
    memcpy(bloqueExtraidoP, imgP + startIndex, mascaraSize);

    return bloqueExtraidoP;
}


//NUEVOS CAMBIOS YA PARA LA COMBINACION
int calcularDiferencia(unsigned int* bloqueTransformado, unsigned int* datosEnmascarados, int n_pixels, bool ignorarDesplazamiento) {
    int diferencia = 0;

    // Iteramos sobre los píxeles
    for (int i = 0; i < n_pixels * 3; ++i) {
        // Imprimir los valores de bloqueTransformado y datosEnmascarados para ver lo que estamos comparando
        /*cout << "Píxel " << i << " bloqueTransformado: " << (int)bloqueTransformado[i]
             << " datosEnmascarados: " << (int)datosEnmascarados[i] << endl;
*/
        // Si ignoramos desplazamientos, no calculamos la diferencia
        if (ignorarDesplazamiento) {
            continue;
        }

        // Si no estamos ignorando, calculamos la diferencia normal
        diferencia += abs((int)bloqueTransformado[i] - (int)datosEnmascarados[i]);
    }

    return diferencia;
}





// Función que prueba cada combinación
int identificarTransformacion(unsigned char* bloque, unsigned char* imgMascara, unsigned int* datosEnmascarados, int dataSize, int n_pixels, unsigned char* bloqueIM) {
    int mejorIndice = -1;
    int menorDiferencia = INT_MAX;

    for (int tipo = 0; tipo < 33; ++tipo) {
        /*if (tipo == 0) {
            unsigned char* transformado = aplicarTransformacionInversa(bloque, tipo, dataSize, bloqueIM);

            for (int i = 0; i < dataSize; ++i) {
                int suma = (int)transformado[i] + (int)imgMascara[i];
                cout << "transformado: " << (int)transformado[i]
                     << " + mascara: " << (int)imgMascara[i]
                     << " = " << suma << " -> " << (suma % 256) << endl;
                transformado[i] = (unsigned char)(suma % 256);
            }

            delete[] transformado; // ¡no olvides liberar la memoria también aquí!
            break;
        }*/

        unsigned char* transformado = aplicarTransformacionInversa(bloque, tipo, dataSize, bloqueIM);

        // Arreglo auxiliar para guardar la suma sin truncar
        unsigned int* sumaCompleta = new unsigned int[dataSize];

        for (int i = 0; i < dataSize; ++i) {
            int suma = (int)transformado[i] + (int)imgMascara[i];
            sumaCompleta[i] = suma;

            /*cout << "Transformado: " << (int)transformado[i]
                 << "  Mascara: " << (int)imgMascara[i]
                 << "  Suma sin recorte: " << suma << endl;*/
        }




        // Verificamos si es un desplazamiento y, si lo es, no calculamos la diferencia
        bool ignorarDesplazamiento = (tipo >= 1 && tipo <= 16);
        // Si es un desplazamiento, no calculamos la diferencia
        int dif = 100000;
        if (!ignorarDesplazamiento) {
            dif = calcularDiferencia(sumaCompleta, datosEnmascarados, n_pixels, false);
        }
        else{
            dif=100000;
        }

//LO VAMOS A QUITARRR
#ifdef DEBUG
        cout << "Tipo " << tipo << " (" << nombresTransformaciones[tipo] << "): diferencia = " << dif << endl;
#endif

        if (dif < menorDiferencia) {
            menorDiferencia = dif;
            mejorIndice = tipo;
            if (dif == 0) {
                delete[] transformado;
                break;  // transformación exacta encontrada
            }
        }

        delete[] transformado;
    }

    return mejorIndice;  // devuelve el índice de la transformación inversa usada
}

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
