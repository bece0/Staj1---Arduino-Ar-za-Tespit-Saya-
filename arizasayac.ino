#include <LiquidCrystal_PCF8574.h>

#include <EEPROM.h>

//#include <LiquidCrystal.h> /* LCD kullanimi icin kutuphane dahil edilmelidir */
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
/* LCDnin baglandigi Arduino pinleri */
LiquidCrystal_PCF8574 lcd(0x3f);

int btndurum = 0;            /*butonun basılıp basılmadığını gösterir*/
int sayac = 0, sayac2 = 0;   /*sayac1 :10luk sayacı ,   sayac2 :10 adet 10luk grupların sayacı*/
int sensorDegeri;            /*sensörün okuduğu degeri gösterir*/
float anlikdeger = 0.0;      /*son süreyi gösterir*/
float onlukORT = 0.0, yuzlukORT = 0.0 ;
float onlukToplam = 0, yuzlukToplam = 0;
int goster = 0 ;            /*buton basma sayısını gösterir*/
float dizi[10];             /*onluk ortalama için*/
float dizi2[10];            /*yuzluk ortalama için*/
float sure = 0;
int ISRsayac = 0;           /*baslangicta 10 saniye kontrolü icin*/
bool hata = 0;  //
int i , j ;

void setup()
{
  pinMode(8, OUTPUT); //İKAZ LEDİ
  pinMode(10, INPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);       /* Kullandigimiz LCDnin sutun ve satir sayisini belirtmeliyiz */
  lcd.setBacklight(255);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Self Test");
  while (true) {
    sensorDegeri = analogRead(A0);

    if (sensorDegeri == 0) {      /*sensöre deger gelmediği sürece sayac arttır*/
      ISRsayac++;
      Serial.print("sayac :");
      Serial.println(ISRsayac);
      lcd.setCursor(10,0);
      lcd.print(ISRsayac);
      delay(1000);


      if (ISRsayac >= 10 ) {      /*baslangıçta 10 saniye boyunca sensöre deger gelmezse*/
        Serial.println("HATA VAR!");
        lcd.print("HATA VAR!");
        hata = 1;
        break;
      }
    } else {
      Serial.println("Calisiyor...");
      Serial.print("*******************\n");
      hata = 0;
      ISRsayac = 0;
      break;
    }
  }
  ISRsayac = 0;

  cli();
  /* Ayarlamaların yapılabilmesi için öncelikle kesmeler durduruldu */
  /* Timer1 kesmesi saniyede bir çalışacak şekilde ayarlanacaktır (1 Hz)*/
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 100;
  /* Bir saniye aralıklar için zaman sayıcısı ayarlandı */
  TCCR1B |= (1 << WGM12);
  /* Adımlar arasında geçen süre kristal hızının 1024'e bölümü olarak ayarlandı */
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  /* Timer1 kesmesi aktif hale getirildi */
  sei();
//
//  //    lcd.begin(16,2);
//  //    lcd.setBacklight(255);
//  //    lcd.home();
//  //    lcd.clear();
//  //    lcd.print("Hello LCD");


  
  lcd.setCursor(0, 0);
  lcd.print("Universal Clinch");
  sei();
}


ISR( TIMER1_COMPA_vect ) {
  sensorDegeri = analogRead(A0);
  while (sensorDegeri == 0 && hata == 0) {   /*dokunmadıgındakı sureyı olcer*/
    sure++;
    sensorDegeri = analogRead(A0);
    if (sensorDegeri > 0) {     /*butona basınca süreyi yazdirir*/

      Serial.print("sure: ");
      Serial.println  ((sure / 1024) / 10);

      dizi[sayac] = ((sure / 1024) / 10);      /*dizi1'in sayac indisine süreyi atar*/
      anlikdeger = dizi[sayac];                /*anlık degere son gelen süreyi atar*/

    //  Serial.print("dizinin el : ");
    //  Serial.println(dizi[sayac]);

      sayac++;
      hata = 0;

      if (sayac > 9) {                        /*dizi1'in 10 elemanının ortalamasını hesaplar*/
        for (int i = 0 ; i < 10; i++) {

          onlukToplam = onlukToplam +  dizi[i];
        }
        onlukORT = onlukToplam / 10;

        onlukToplam = 0;
        dizi2[sayac2] = onlukORT;
        sayac2++;

        Serial.print("onlukOrt: ");
        Serial.println(onlukORT);
        Serial.print("*******************\n");

        if (sayac2 > 9) {                     /*toplam 10 grup oluştugunda bunlarin ortalamalarini alır*/
          for (int j = 0 ; j < 10 ; j++) {
            yuzlukToplam = yuzlukToplam +  dizi2[j];
          }
          yuzlukORT = yuzlukToplam / 10;
          Serial.print("yuzlukort: ");
          Serial.println(yuzlukORT);
          Serial.print("*******************\n");

          yuzlukToplam = 0;
          sayac2 = 0;
        }

        sayac = 0;
      }
      sure = 0;
      break;
    }
    else {                                 /*Tele dokunmadıysa*/
      if ( (sure / 1024) / 10 > 10) {      /*10 saniye boyunca deger gelmazse hata verir*/
        Serial.println("!!!!!!! HATA VAR !!!!!!! ");
        hata = 1;
        sure = 0;
      }
    }

  }
}

void loop()
{

  btndurum = digitalRead(10);
  if (btndurum == HIGH) {                         /*butona basıldıysa*/

    Serial.print("basildi\n");

    while (1) {
      btndurum = digitalRead(10);

      if (btndurum == LOW  && goster < 4 ) {      /*butondan cekilmisse gosteri arttır*/
        goster++;
        break;
      }
    }
    if (goster >= 3) {                        /*butona 3. basışta gosteri sıfırlar*/
      goster = 0;
    }
  }


  switch (goster) {
    case 0:
      if (hata == 1) {             /*hata varsa*/
        digitalWrite(8, HIGH);     /*led yakar*/
        //lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Hata var        ");
        lcd.setCursor(0, 1);
        lcd.print("Con.Tube Tikali");
      }
      else {      /*hata yoksa*/
        digitalWrite(8, LOW);
        lcd.setCursor(1, 1);
        lcd.print("  Anlik : ");
        lcd.print(anlikdeger);

        break;

      }
      break;


    case 1:
      lcd.setCursor(2, 1);
      lcd.print("Son 10 : ");
      lcd.print(onlukORT);
      digitalWrite(8, LOW);
      break;

    case 2:
      lcd.setCursor(1, 1);
      lcd.print(" Son 100 :");
      lcd.print(yuzlukORT);
      digitalWrite(8, LOW);
      break;

  }
}

