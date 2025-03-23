typedef struct data {
  char grupo[20];
  int origen;
  int destino;
  char mensaje[100];

} msg;

typedef struct grupo {
  char codigo[20];
  int* ids;

} grupo;