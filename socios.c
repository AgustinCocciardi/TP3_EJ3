#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define DNI 10
#define FECHA 12
#define DIA 10
#define REGISTROS 200
#define VALOR 30

typedef struct datos
{
    char pagoDni[REGISTROS][DNI];
    char pagoFecha[REGISTROS][FECHA];
    char asistenciaDni[REGISTROS][DNI];
    char asistenciaDia[REGISTROS][DNI];
} registros;

typedef struct socio
{
    char nombre[20];
    char apellido[30];
    char dni[9];
    char deporte[9];
    char dia[10];
} estructuraSocios;


int main(int argc, char* argv[]){
    char *ayuda="-Help";    //Muestro la ayuda
    if (argc == 2 && strcmp(argv[1],ayuda) == 0) //Muestro la ayuda al usuario
    {
        printf("\nEste programa tiene la funcionalidad de leer un archivo de socios y recibir información a procesar.");
        printf("\nLo que debe hacer este programa es leer del archivo de socios y procesar los datos que recibirá de los procesos Pagos y Asistencia");
        printf("\nEste debe ser el primer proceso en correr");
        printf("\nEjemplo de ejecución: ./Pagos");
        printf("\n");
        exit(3);
    }

    //Datos del club//
    char *futbolDia1 = "Lunes";
    char *futbolDia2 = "Miercoles";
    char *voleyDia1 = "Martes";
    char *voleyDia2 = "Jueves";
    char *basquetDia = "Viernes";
    char *natacionDia = "Sabado";
    int futbolPrecio = 1000;
    int voleyPrecio = 1200;
    int basquetPrecio = 1300;
    int natacionPrecio = 1800;

    //Abro el archivo y valido que haya podido abrirlo
    char *socios="socios.txt";

    FILE *archivoSocios;
    archivoSocios=fopen(socios,"r");

    if (archivoSocios == NULL)
    {
        printf("\nEl archivo no existe o no tiene permisos de lectura\n");
        exit(2);
    }

    //Me va a servir para leer del archivo
    char delimitador[]=";\n";
    char palabra[85];

    char *dni;
    char *nombre;
    char *apellido;
    char *nombreDeporte;
    char *diaAsistencia;
    
    int contador=0;
    while (feof(archivoSocios) == 0)
    {
        fgets(palabra,85,archivoSocios);
        contador++;
        /*nombre= strtok(palabra,delimitador);
        apellido= strtok(NULL,delimitador);
        dni= strtok(NULL,delimitador);
        nombreDeporte= strtok(NULL,delimitador);
        diaAsistencia= strtok(NULL,delimitador);
        printf("Nombre y apellido: %s %s\t DNI: %s\t Deporte y Dia: %s %s\n", nombre, apellido, dni,nombreDeporte,diaAsistencia);*/
    }
    rewind(archivoSocios);

    //printf("Hay %d registros en el archivo", contador);

    estructuraSocios sociosClub[contador];
    int i=0;
    while (feof(archivoSocios) == 0)
    {
        fgets(palabra,85,archivoSocios);
        nombre= strtok(palabra,delimitador);
        apellido= strtok(NULL,delimitador);
        dni= strtok(NULL,delimitador);
        nombreDeporte= strtok(NULL,delimitador);
        diaAsistencia= strtok(NULL,delimitador);
        strcpy(sociosClub[i].nombre,nombre);
        strcpy(sociosClub[i].apellido,apellido);
        strcpy(sociosClub[i].dni,dni);
        strcpy(sociosClub[i].deporte,nombreDeporte);
        strcpy(sociosClub[i].dia,diaAsistencia);
        i++;
    }

    /*
    for (int i = 0; i < contador; i++)
    {
        printf("Nombre y apellido: %s %s\t DNI: %s\t Deporte y Dia: %s %s\n", sociosClub[i].nombre, sociosClub[i].apellido, sociosClub[i].dia, sociosClub[i].deporte, sociosClub[i].dia);
    }
    */
    
    //Cierro el archivo
    fclose(archivoSocios);

    //Para esta parte, ya tengo los datos de mis socios en la estructura sociosClub. Voy a empezar a abrir los recursos compartidos

    key_t Clave;                    //clave para recursos compartidos
    int Id_Memoria;                 //Identificador de Memoria

    Clave = ftok("/bin/ls",VALOR);          //Pido una clave para recursos compartidos y verifico que haya podido recibirla
    if (Clave == (key_t) -1)
	{
		printf("No consigo clave para memoria compartida\n");
		exit(0);
    }

    Id_Memoria = shmget (Clave, sizeof(registros *)*1000, IPC_CREAT | 0666); //Pido ID para memoria compartida
    if (Id_Memoria == -1)
	{
		printf("No consigo Id para memoria compartida\n");
		exit (0);
    }

    registros* Memoria = (registros *)shmat (Id_Memoria, (registros *)0, 0);
    if(Memoria == NULL){
        printf("No pude conseguir memoria compartida\n");
        exit(0);
    }

    //Declaro los nombres de mis semaforos
    sem_t *semutex;
    sem_t *sepagos;
    sem_t *seasistencia;
    sem_t *sesocios;

    //Creo e inicializo los semáforos
    semutex = sem_open("/mutex",O_CREAT|O_EXCL,0644,1);               //Inicializo el Mutex en 1
    sepagos = sem_open("/pagos",O_CREAT|O_EXCL,0644,1);               //Inicializo el semaforo Pagos en 1
    seasistencia = sem_open("/asistencia",O_CREAT|O_EXCL,0644,0);     //Inicializo el semaforo Asistencia en 0
    sesocios = sem_open("/socios",O_CREAT|O_EXCL,0644,0);             //Inicializo el semaforo Socios en 0

    //Hago un P() a mi semaforo de socios. Como está inicializado en 0, me quedo esperando a que el proceso asistencia haga un V()
    sem_wait(sesocios);
    sem_wait(semutex);  //Tomo el Mutex para leer de memoria compartida

    //Calcular el monto mensual (Revisar descuentos)
    int montoEnero=0;
    int montoFebrero=0;
    int montoMarzo=0;
    int montoAbril=0;
    int montoMayo=0;
    int montoJunio=0;
    int montoJulio=0;
    int montoAgosto=0;
    int montoSeptiembre=0;
    int montoOctubre=0;
    int montoNoviembre=0;
    int montoDiciembre=0;
    int j=0;
    char auxDocDni;
    char auxFecha[11];
    char *anio;
    char *mes;
    char *dia;
    char delim[2] = "-";
    char *sport;
    float monto=0;
    int descuento;
    while (strcmp(Memoria->pagoDni[j],"00") != 0)
    {
        descuento = 0;
        strcpy(auxDocDni,Memoria->pagoDni[j]);
        strcpy(auxFecha,Memoria->pagoFecha[j]);
        anio = strtok(auxFecha,delim);
        mes = strtok(NULL,delim);
        dia = strtok(NULL,delim);
        for (int i = 0; i < contador; i++)
        {
            if ((strcmp(auxDocDni,sociosClub[i].dni)) == 0)
            {
                strcpy(sport,sociosClub[i].deporte);
                break;
            }
        }
        if (strcmp(dia,"01") == 0 || strcmp(dia,"02") == 0 || strcmp(dia,"03") == 0 || strcmp(dia,"04") == 0 || strcmp(dia,"05") == 0 || strcmp(dia,"06") == 0 || strcmp(dia,"07") == 0 || strcmp(dia,"08") == 0 || strcmp(dia,"09") == 0 || strcmp(dia,"10") == 0)
        {
            descuento=1;
        }
        if (strcmp(sport,"Futbol"))
        {
            monto=futbolPrecio;
        }
        if (strcmp(sport,"Voley"))
        {
            monto=voleyPrecio;
        }
        if (strcmp(sport,"Basquet"))
        {
            monto=basquetPrecio;
        }
        if (strcmp(sport,"Natacion"))
        {
            monto=natacionPrecio;
        }

        if (descuento == 0)
        {
            monto-=(monto*10/100);
        }
        
        if (strcmp(mes,"01") == 0)
        {
            montoEnero+=monto;
        }
        else if (strcmp(mes,"02") == 0)
        {
            montoFebrero+=monto;
        }
        else if (strcmp(mes,"03") == 0)
        {
            montoMarzo+=monto;
        }
        else if (strcmp(mes,"04") == 0)
        {
            montoAbril+=monto;
        }
        else if (strcmp(mes,"05") == 0)
        {
            montoMayo+=monto;
        }
        else if (strcmp(mes,"06") == 0)
        {
            montoJunio+=monto;
        }
        else if (strcmp(mes,"07") == 0)
        {
            montoJulio+=monto;
        }
        else if (strcmp(mes,"08") == 0)
        {
            montoAgosto+=monto;
        }
        else if (strcmp(mes,"09") == 0)
        {
            montoSeptiembre+=monto;
        }
        else if (strcmp(mes,"10") == 0)
        {
            montoOctubre+=monto;
        }
        else if (strcmp(mes,"11") == 0)
        {
            montoNoviembre+=monto;
        }
        else
        {
            montoDiciembre+=monto;
        }
        j++;
    }
    if (montoEnero > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoEnero);
    }
    if (montoFebrero > 0)
    {
        printf("Monto cobrado en febrero: %d\n", montoFebrero);
    }
    if (montoMarzo > 0)
    {
        printf("Monto cobrado en marzo: %d\n", montoMarzo);
    }
    if (montoAbril > 0)
    {
        printf("Monto cobrado en abril: %d\n", montoAbril);
    }
    if (montoMayo > 0)
    {
        printf("Monto cobrado en Mayo: %d\n", montoMayo);
    }
    if (montoJunio > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoJunio);
    }
    if (montoJulio > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoJulio);
    }
    if (montoAgosto > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoAgosto);
    }
    if (montoSeptiembre > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoSeptiembre);
    }
    if (montoOctubre > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoOctubre);
    }
    if (montoNoviembre > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoNoviembre);
    }
    if (montoDiciembre > 0)
    {
        printf("Monto cobrado en enero: %d\n", montoDiciembre);
    }
    
    //Detectar asociados que no pagaron cuota mensual
    char *socioActual;
    char *auxiliarFecha;
    char *auxiliarAnio;
    char *auxiliarMes;
    for (int i = 0; i < contador; i++)
    {
        strcpy(socioActual,sociosClub[i].dni);
        int b=0;
        int enero = 0, febrero = 0, marzo = 0, abril = 0, mayo = 0, junio = 0, julio = 0, agosto = 0, septiembre = 0, octubre = 0, noviembre = 0, diciembre = 0;
        while (strcmp(Memoria->asistenciaDni[b],"00") != 0)
        {
            if (strcmp(Memoria->pagoDni[b],socioActual) == 0)
            {
                strcpy(auxiliarFecha,Memoria->pagoDni[b]);
                auxiliarAnio = strtok(auxiliarFecha,delim);
                auxiliarMes = strtok(NULL,delim);
                if (strcmp(auxiliarMes,"01") == 0)
                {
                    enero = 1;
                }
                else if (strcmp(auxiliarMes,"02") == 0)
                {
                    febrero = 1;
                }
                else if (strcmp(auxiliarMes,"03") == 0)
                {
                    marzo = 1;
                }
                else if (strcmp(auxiliarMes,"04") == 0)
                {
                    abril = 1;
                }
                else if (strcmp(auxiliarMes,"05") == 0)
                {
                    mayo = 1;
                }
                else if (strcmp(auxiliarMes,"06") == 0)
                {
                    junio = 1;
                }
                else if (strcmp(auxiliarMes,"07") == 0)
                {
                    julio = 1;
                }
                else if (strcmp(auxiliarMes,"08") == 0)
                {
                    agosto = 1;
                }
                else if (strcmp(auxiliarMes,"09") == 0)
                {
                    septiembre = 1;
                }
                else if (strcmp(auxiliarMes,"10") == 0)
                {
                    octubre = 1;
                }
                else if (strcmp(auxiliarMes,"11") == 0)
                {
                    noviembre = 1;
                }
                else
                {
                    diciembre = 1;
                }
                
            }
            
            b++;
        }
        if (enero == 0)
        {
            printf("El socio %s no abono el mes de enero\n", socioActual);
        }
        if (febrero == 0)
        {
            printf("El socio %s no abono el mes de febrero\n", socioActual);
        }
        if (marzo == 0)
        {
            printf("El socio %s no abono el mes de marzo\n", socioActual);
        }
        if (abril == 0)
        {
            printf("El socio %s no abono el mes de abril\n", socioActual);
        }
        if (mayo == 0)
        {
            printf("El socio %s no abono el mes de mayo\n", socioActual);
        }
        if (junio == 0)
        {
            printf("El socio %s no abono el mes de junio\n", socioActual);
        }
        if (julio == 0)
        {
            printf("El socio %s no abono el mes de julio\n", socioActual);
        }
        if (agosto == 0)
        {
            printf("El socio %s no abono el mes de agosto\n", socioActual);
        }
        if (septiembre == 0)
        {
            printf("El socio %s no abono el mes de septiembre\n", socioActual);
        }
        if (octubre == 0)
        {
            printf("El socio %s no abono el mes de octubre\n", socioActual);
        }
        if (noviembre == 0)
        {
            printf("El socio %s no abono el mes de noviembre\n", socioActual);
        }
        if (diciembre == 0)
        {
            printf("El socio %s no abono el mes de diciembre\n", socioActual);
        }
        
    }
    

    //Detectar asistencias en dias que no corresponden
    int a=0;
    char *auxDni;
    char *auxDia;
    while (strcmp(Memoria->asistenciaDni[a],"00") != 0)
    {
        strcpy(auxDni,Memoria->asistenciaDni[a]);
        strcpy(auxDia,Memoria->asistenciaDia[a]);
        for (int i = 0; i < contador; i++)
        {
            if (strcmp(auxDni,sociosClub[i].dni) == 0)
            {
                if (strcmp(auxDia,sociosClub[i].dia) != 0)
                {
                    printf("El socio %s asiste el dia %s cuando debería asistir el dia %s", auxDni,auxDia,sociosClub[i].dia);
                }
                break;
            }
        }
        a++;
    }
    

    //Cierro los semaforos
    sem_close(semutex);
    sem_close(sepagos);
    sem_close(seasistencia);
    sem_close(sesocios);

    //Elimino los semaforos
    sem_unlink("/mutex");
    sem_unlink("/pagos");
    sem_unlink("/asistencia");
    sem_unlink("/socios");

}
