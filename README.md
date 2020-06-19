# TP3_EJ3

El club de barrio “La Juanita” está teniendo problemas para llevar a cabo la correcta contabilidad
del pago
de las cuotas de sus asociados, así como el control de la correcta asistencia en los días
correspondientes
debido al incremento de socios que se ha dado en el último año. Para automatizar de la forma más
económica posible lo contactan a usted para brindar solución a dicha problemática.
El club mantiene los siguientes archivos (Todos los archivos separan registros por punto y coma
(;)):
• Socios: contiene: Nombre;Apellido;DNI;NombreDelDeporte;DíaAsistencia. Por ahora no está
permitido anotarse a más de un deporte y además solo se puede asistir un día a la semana.
o El campo nombreDelDeporte puede tomar los valores: “Futbol”,”Voley”,”Basquet” y
“Natación” (respetando las minúsculas y mayúsculas)
o El campo Día puede tomar los valores “Lunes, Martes, Miercoles, Jueves, Viernes,
Sabado y Domingo”
• Pagos: Archivo separado por punto y coma que posee el siguiente formato: DNI;Fecha. Si el
pago se realiza del 1 al 10 se realiza un descuento del 10%.
o Formato de fecha: YYYY-MM-DD
• Asistencia: Archivo separado por Punto y Coma que posee el siguiente formato:
DNI;DiaAsistencia
o El campo DiaAsistencia toma los mismos valores que el del archivo Socios
En cuanto a los deportes que se practican, se tiene la siguiente información (no se encuentra en
ningún archivo, puede estar como valores en el código):
Deported Días posibles Valor de la cuota
Fútbol Lunes - Miércoles $ 1.000
Vóley Martes - Jueves $ 1.200
Básquet Viernes $ 1.300
Natación Sábado $ 1.800
Lo que necesita principalmente el club es:
1. Calcular automáticamente el monto total cobrado en el mes (contemplando los
descuentos).
2. Detectar asociados que no hayan pagado la cuota mensual.
3. Detectar asistencias en días que no corresponden.
Dado que en el club utilizan Linux como sistema operativo, después de la etapa de análisis con
el personal
del club, se definen los siguientes lineamientos:
1. Habrá dos procesos productores, uno leerá la información del archivo Pagos y el otro del
archivo Asistencia, ambos enviarán la información a través de memoria compartida a un
proceso consumidor, que será el encargado de realizar el proceso correspondiente para
responder a las necesidades del club, mostrando los resultados por pantalla.
2. Tiene que existir alternancia estricta en el envío de información entre los procesos
productores.
