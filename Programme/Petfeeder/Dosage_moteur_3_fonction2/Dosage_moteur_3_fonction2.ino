
//inclure les librairies suivantes
#include <Stepper.h>
#include <LiquidCrystal.h>
#include <Wire.h>

/*************************************************************/

// definition des paramètre du STEPPER

const int stepsPerRevolution = 200;  // definit le nombre de pas du moteur 
const int AvanceFood = stepsPerRevolution/4; // defini le nombre de pas pour avancer la nourriture
const int ReculeFood = AvanceFood/3; // defini le nombre de pas pour reculer la nourriture
int QuantiteMatin = 10; // quantite de tour de vis le Matin
int doseMultiply = 5; //
int SpeedRemplissage = 30; // vitesse de rotation de la vis
int hasBeenFeed = 0;


//Definir les branchements du driver sur la carte arduino
Stepper myStepper(stepsPerRevolution, A2,A3,A4,A5);

// Gestion du temps
int timeHasChanged = 0; // bool dire si les minutes ont changé
int quantityHasChanged = 0;
int oldDiffMin; //l'anciennes différence de minutes;

/*************************************************************/

// definition des paramètres de l'ecran LCD

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
char MenuOn     = 0;
#define btnRight   0
#define btnUp      1
#define btnDown    2
#define btnLeft    3
#define btnSelect  4
#define btnNone    5

#define LCD_STR_LEN 17  // 16 caractère + '\0' de fin de chaîne (au cas où)

// 5/6) Lire/Déterminer l'état de tous les boutons
int read_LCD_buttons(){
  adc_key_in = analogRead(0);      // read the value from the sensor 
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (adc_key_in > 1000) return btnNone; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)   return btnRight;  
  if (adc_key_in < 200)  return btnUp; 
  if (adc_key_in < 350)  return btnDown; 
  if (adc_key_in < 550)  return btnLeft; 
  if (adc_key_in < 850)  return btnSelect;  
  return btnNone;  // when all others fail, return this...
}

typedef struct _Menu {
  char title[LCD_STR_LEN];
  void (* selectAction) ();
  void (* leftAction) ();
  void (* upAction) ();
  void (* downAction) ();
  void (* rightAction) ();
  void (* resetAction) ();
  struct _Menu *selectMenu;
  struct _Menu *leftMenu;
  struct _Menu *upMenu;
  struct _Menu *downMenu;
  struct _Menu *rightMenu;
  struct _Menu *resetMenu;
  struct _Time *Time;
} Menu;

// 1/6) Création des menus
Menu menu10 = {
  "HEURE:"
};

Menu menu20 = {
  "Reglage heure"
};

Menu menu30 = {
  "Heure dosage 1"
};

Menu menu40 = {
  "Heure dosage 2"
};

Menu menu50 = {
  "Heure dosage 3"
};

Menu menu60 = {
  "Dosage manuel"
};

Menu menu70 = {
  "Quantite dosage"
};

Menu* currentMenu;
Menu* oldMenu;

typedef struct _Time {
  int hour;
  int min;
  unsigned long millisTime;
} Time;

Time isSetTime = {0,0,0}; //time set in setTime it is fix
Time time1;// real time
Time alarm1 = {8,0};
Time alarm2 ={16,0};
Time alarm3 ={24,0};

// 2/6) Déclaration des actions utilisé par les menus
// (ce ne sont que des exemples)
void setTime(){
  setHour();
  setMin();
  hasBeenFeed = 0; //remet à zero le paramètre qui accepte le déclenchement du petFeeder
  if(currentMenu == &menu20){
      isSetTime.hour = time1.hour;
      isSetTime.min = time1.min;
      isSetTime.millisTime = millis();
      oldDiffMin = 0;
  }
}

void setHour() {
  int b = 0;
  int period = 100;
  int totalDuration = 1000;

  delay(500);
  while(b<totalDuration){
    Serial.println(b);
    lcd_key = read_LCD_buttons();
    if(lcd_key==btnUp){
      if(currentMenu->Time->hour==23){
        currentMenu->Time->hour = 0;
      } 
      else{
        currentMenu->Time->hour++;
      }
      lcd_key = 0;
      b = 0;

      //print time
      lcd.setCursor(0,1);
      showHour(currentMenu->Time->hour, currentMenu->Time->min);

      //eviter les rebonds
      delay(500);
    }
    if(lcd_key==btnDown){
      if(currentMenu->Time->hour==0){
        currentMenu->Time->hour = 23;
      } 
      else{
        currentMenu->Time->hour--;
      }
      lcd_key = 0;
      b = 0;

      //print time
      lcd.setCursor(0,1);
      showHour(currentMenu->Time->hour, currentMenu->Time->min);

      //eviter les rebonds
      delay(500);
    }
    if((b/period)%2 == 0){
      lcd.setCursor(0,1);
      if(currentMenu->Time->hour == 0){
        lcd.print(" ");
      }else{
        for (int a = 0; a < floor (log10 (abs (currentMenu->Time->hour))) + 1; a++){
          lcd.print(" ");
        }
      }
    } else{
      lcd.setCursor(0,1);
      showHour(currentMenu->Time->hour, currentMenu->Time->min);
    }
    if(lcd_key==btnSelect){
      b = totalDuration;
      delay(500); 
    }
    b++;
  }
  lcd.setCursor(0,1);
  showHour(currentMenu->Time->hour, currentMenu->Time->min);
}
void setMin() {
  int b = 0;
  int period = 100;
  int totalDuration = 1000;

  delay(500);
  while(b<totalDuration){
    Serial.println(b);
    lcd_key = read_LCD_buttons();
    if(lcd_key==btnUp){
      if(currentMenu->Time->min==59){
        currentMenu->Time->min = 0;
      } 
      else{
        currentMenu->Time->min++;
      }
      lcd_key = 0;
      b = 0;

      //print time
      lcd.setCursor(0,1);
      showHour(currentMenu->Time->hour, currentMenu->Time->min);

      //eviter les rebonds
      delay(500);
    }
    if(lcd_key==btnDown){
      if(currentMenu->Time->min==0){
        currentMenu->Time->min = 59;
      } 
      else{
        currentMenu->Time->min--;
      }
      lcd_key = 0;
      b = 0;

      //print time
      lcd.setCursor(0,1);
      showHour(currentMenu->Time->hour, currentMenu->Time->min);

      //eviter les rebonds
      delay(500);
    }

    //Déplace le curseur même si heure = 0
    if(currentMenu->Time->hour == 0){
      lcd.setCursor(2,1);
    }
    else{
      //déplace le curseur pour écrire à l'emplacement minute
      lcd.setCursor(floor(log10 (abs (currentMenu->Time->hour))) + 2, 1); // ne fonctionne pas =0   
    }

    if((b/period)%2 == 0){
      lcd.print("  ");
    } else{
      lcd.setCursor(0,1);
      showHour(currentMenu->Time->hour, currentMenu->Time->min);
    }
    if(lcd_key==btnSelect){
      b = totalDuration;
      delay(500); 
    }
    b++;
  }
  lcd.setCursor(0,1);
  showHour(currentMenu->Time->hour, currentMenu->Time->min);
}

void Feeding() {
  remplissage(QuantiteMatin*doseMultiply);
}

void remplissage(int volume){ // Int volume = permet de remplacer la variable volume par "QuantiteMatin" ou "QuantiteSoir" selon l'appel

    int CompteurTour = 0; // compte le nombre de tour de vis
  
  while(CompteurTour < volume){  
    myStepper.step(-AvanceFood); //fait avancer pendant XXXX millisecondes
    delay(100);
    myStepper.step(ReculeFood); //fait reculer pendant XXXX millisecondes
    delay(100); 
    CompteurTour++;  //on incrémente le compteur
  }
    digitalWrite(A2,LOW);
    digitalWrite(A3,LOW);
    digitalWrite(A4,LOW);
    digitalWrite(A5,LOW);
}

void IncreaseQ(){
  if(doseMultiply<10){
    doseMultiply=doseMultiply + 1;
  }
  quantityHasChanged = 1;
}

void DecreaseQ(){
  if(doseMultiply>1){
    doseMultiply=doseMultiply - 1;
  }
  quantityHasChanged = 1;
}

void LCD(char texte[]){
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print(texte); // Afficher le texte
}

void changeMenu(){
  // 6/6) On execute de ce qu'on doit faire, si un des boutons est appuyé
  if (lcd_key == btnSelect) {
    if (currentMenu->selectAction) currentMenu->selectAction();
    if (currentMenu->selectMenu) currentMenu = currentMenu->selectMenu;
    delay(500);
  }
  else if (lcd_key == btnLeft) {
    if (currentMenu->leftAction) currentMenu->leftAction();
    if (currentMenu->leftMenu) currentMenu = currentMenu->leftMenu;
    delay(500);
  }
  else if (lcd_key == btnUp) {
    if (currentMenu->upAction) currentMenu->upAction();
    if (currentMenu->upMenu) currentMenu = currentMenu->upMenu;
    delay(500);
  }
  else if (lcd_key == btnDown) {
    if (currentMenu->downAction) currentMenu->downAction();
    if (currentMenu->downMenu) currentMenu = currentMenu->downMenu;
    delay(500);
  }
  else if (lcd_key == btnRight) {
    if (currentMenu->rightAction) currentMenu->rightAction();
    if (currentMenu->rightMenu) currentMenu = currentMenu->rightMenu;
    delay(500);
  }
//   else if (lcd_key == btnReset) {
//     if (currentMenu->resetAction) currentMenu->resetAction();
//     if (currentMenu->resetMenu) currentMenu = currentMenu->resetMenu;
//  }
}

//Print a basic header on Row 1.
void clearPrintTitle() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("PET FEEDER");
  lcd.setCursor(0,1); 
}

void showHour(int hour,int min){
  char H1[2];
  sprintf(H1, "%d", hour);
  lcd.print(H1);
  lcd.print("h");
  char M1[2];
  sprintf(M1, "%d", min);
  if(floor (log10 (abs (min))) + 1 < 2){
    lcd.print("0");
  }
  lcd.print(M1);
  lcd.print(" ");
}

void showMainMenu(){
  // definition des paramètre du STEPPER
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0,0);
  lcd.print("HEURE: "); // Afficher l'heure actuelle
  showHour(time1.hour, time1.min);
  
  lcd.setCursor(0,1);
  lcd.print("1-"); // Afficher les heures programmées  
  showHour(menu30.Time->hour, menu30.Time->min);

  lcd.setCursor(9,1);
  lcd.print("2-");
  showHour(menu40.Time->hour, menu40.Time->min);
}

void updateTime(){
  
  unsigned long diffMills = millis();
  diffMills = diffMills - isSetTime.millisTime;
  int diffMin = 0;
  int diffHour = 0;

  diffMills = diffMills/60000;
  diffMin = (int) diffMills;
  
  if(diffMin > oldDiffMin){ // Vérifier si l'heure a changé
    timeHasChanged = 1;
    oldDiffMin = diffMin;

    if((isSetTime.min+diffMin) > 59){
      diffHour = (isSetTime.min+diffMin)/60;
    }
    int addDay = (isSetTime.hour + diffHour)/24;
    time1.hour = isSetTime.hour + diffHour - 24*addDay;
    time1.min = isSetTime.min + diffMin - 60*diffHour;
  }

  if(hasBeenFeed == 1 && timeHasChanged == 1){
    hasBeenFeed = 0;
  }
}

/*************************************************************/
void setup(){
/*************************************************************/

  // 4/6) Setup des liens entre menu
  menu10.leftMenu = &menu70;
  menu10.rightMenu = &menu20;

  menu20.leftMenu = &menu10;
  menu20.rightMenu = &menu30;
  menu20.Time = &time1;
  menu20.selectAction = setTime;

  menu30.leftMenu = &menu20;
  menu30.rightMenu = &menu40; 
  menu30.Time = &alarm1;
  menu30.selectAction = setTime;

  menu40.leftMenu = &menu30;
  menu40.Time = &alarm2;
  menu40.rightMenu = &menu50;
  menu40.selectAction = setTime;

  menu50.leftMenu = &menu40;
  menu50.Time = &alarm3;
  menu50.rightMenu = &menu60;
  menu50.selectAction = setTime;

  menu60.leftMenu = &menu50;
  menu60.rightMenu = &menu70;
  menu60.upAction = Feeding;

  menu70.leftMenu = &menu60;
  menu70.rightMenu = &menu10;
  menu70.upAction = IncreaseQ;
  menu70.downAction = DecreaseQ;

  // Initialisation du menu au démarrage
  currentMenu = &menu10;

  // definition des paramètre du STEPPER  
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);
  pinMode(A5,OUTPUT);
  myStepper.setSpeed(SpeedRemplissage); //mettre la vitesse de rotation à 30tr/min

  //initialisation de l'heure
  time1.hour = isSetTime.hour;
  time1.min = isSetTime.min;
  isSetTime.millisTime = millis();

  Serial.begin(9600);
}


void loop() {
  lcd_key = read_LCD_buttons();
  // Affichage du texte du menu courant:
  updateTime();
  // On exécute ce que l'on souhaite en plus
  // Pour un affichage supplémentaire (par exemple)
  if (currentMenu == &menu10 && (oldMenu != &menu10 || timeHasChanged)) {
    showMainMenu();
    timeHasChanged = 0;
  }
  if(currentMenu == &menu20 && oldMenu != &menu20) {
    LCD(currentMenu->title);
    lcd.setCursor(0,1);
    showHour(currentMenu->Time->hour, currentMenu->Time->min);
  }
  if(currentMenu == &menu30 && oldMenu != &menu30) {
    LCD(currentMenu->title);
    lcd.setCursor(0,1);
    showHour(currentMenu->Time->hour,currentMenu->Time->min);
  }
  if (currentMenu == &menu40 && oldMenu != &menu40) {
    // N'est exécuté que si le menu courant est menu40
    LCD(currentMenu->title);
    lcd.setCursor(0,1);
    showHour(currentMenu->Time->hour,currentMenu->Time->min);
  }
  if (currentMenu == &menu50 && oldMenu != &menu50) {
    // N'est exécuté que si le menu courant est menu50
    LCD(currentMenu->title);
    lcd.setCursor(0,1);
    showHour(currentMenu->Time->hour,currentMenu->Time->min);
  }

  if (currentMenu == &menu60 && oldMenu != &menu60) {
    // N'est exécuté que si le menu courant est menu40
    LCD(currentMenu->title);
    lcd.setCursor(0,1);
    lcd.print("Press Up");
  }
  
  if (currentMenu == &menu70 && (oldMenu != &menu70 || quantityHasChanged)) {
    // N'est exécuté que si le menu courant est menu70
    LCD(currentMenu->title);
    lcd.setCursor(0,1);
    for(int i = 0; i < doseMultiply ;i++){
      lcd.print("O");
    }
    if(quantityHasChanged){
      quantityHasChanged = 0;
    }
  }

  // Si l'heure est égale à l'heure de l'alarme 1 -> nourrir
  if((menu20.Time->min == menu30.Time->min || menu20.Time->min == menu40.Time->min || menu20.Time->min == menu50.Time->min) && hasBeenFeed == 0){
    if(menu20.Time->hour == menu30.Time->hour || menu20.Time->hour == menu40.Time->hour || menu20.Time->hour == menu50.Time->hour){
      Feeding();
      hasBeenFeed = 1;
    }
  }

  oldMenu = currentMenu;
  //Changement de menu ou action
  changeMenu();
}
