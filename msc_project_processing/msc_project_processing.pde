/* Project: MSc project, Human-Computer Interaction programme, University College London
   Required HW: Arduino Mega 2560, 1366x768 external LCD display */

import processing.serial.*;
import processing.video.*;


Serial arduinoPort;  // create object from Serial class
String arduinoData;     // data received from the serial port


PImage[] storyImages = new PImage[50]; // stores all content screens
PImage currentImage;

Movie[] storyMovies = new Movie[3]; // stores all videos

int newScreenNo = 0;
int currentScreenNo = 0;
int previousScreenNo = 0;

Table dataLogTable; // table for logging incoming Arduino Data

boolean playFlag = false;
boolean showPromptToFinish = false;

// Countdown - automatic restart
PFont font;
String time = "30";
int t;
int interval = 30;
int timeFinishScreenSet;

// Shake hands with Arduino
boolean firstContact = false;

void setup()
{  

  //String portName = Serial.list()[0]; // list all available ports if needed
  arduinoPort = new Serial(this, "COM3", 115200);
  arduinoPort.bufferUntil('\n');  // shake hands with Arduino
  // arduinoPort.clear();

  font = createFont("Arial", 27);
  fullScreen(2);

  // Load images
  for (int i = 0; i < storyImages.length; i++) {
    storyImages[i] = loadImage(i + ".png");
  }

  // Load videos 
  for (int i = 0; i < storyMovies.length; i++) {
    storyMovies[i] = new Movie(this, i + ".mp4");
  }

  // Create a table for data logging
  dataLogTable= new Table(); 
  dataLogTable.addColumn("year");
  dataLogTable.addColumn("month");
  dataLogTable.addColumn("day");
  dataLogTable.addColumn("hour");
  dataLogTable.addColumn("minute");
  dataLogTable.addColumn("second");
  dataLogTable.addColumn("interaction");
}

void draw() {

  if (currentScreenNo == 7 && playFlag==true) {   // only display the video on screen if it is playing (do not draw it if the user comes back to this page from another screen)
    image(storyMovies[0], 0, 440, 768, 430);

    //reset when playback has finished
    if (storyMovies[0].time() == storyMovies[0].duration()) {
      storyMovies[0].stop();
      // send message to Arduino to switch off the Stop button
      arduinoPort.write('0');
    }
  } else if (currentScreenNo == 23 && playFlag==true) {
    image(storyMovies[1], 0, 400, 768, 432);

    //reset when playback has finished
    if (storyMovies[1].time() == storyMovies[1].duration()) {
      storyMovies[1].stop();
      // send message to Arduino to switch off the Stop button
      arduinoPort.write('0');
    }
  } else if (currentScreenNo == 35 && playFlag==true) {
    image(storyMovies[2], 0, 430, 768, 432);

    //reset when playback has finished
    if (storyMovies[2].time() == storyMovies[2].duration()) {
      storyMovies[2].stop();
      // send message to Arduino to switch off the Stop button
      arduinoPort.write('0');
    }
  }
  if (showPromptToFinish == true && (currentScreenNo == 8 || currentScreenNo == 14 || currentScreenNo == 23 || currentScreenNo == 29 || currentScreenNo == 35 || currentScreenNo == 41)) {

    image(storyImages[49], 0, 1172); // overlay "Explore another topic" part of the screen with a Finish prompt when if user is on the very last screen, regardless on experiment (could be any of the last screen within each story unit)
  }
  if (currentScreenNo == 46) {
    background(0);
    image(storyImages[46], 0, 0);
    int countDown = millis() - timeFinishScreenSet;

    t = interval-int(countDown/1000);
    time = nf(t, 2);

    textSize(26); 
    fill(255);
    text(time, 563, 1150);
  }

  // If data is available, read it and store it in val
  while ( arduinoPort.available() > 0) 
  {  
    arduinoData = arduinoPort.readStringUntil('\n');

    if (arduinoData != null && arduinoData !="") {
      println(arduinoData); // for debugging
      arduinoData = arduinoData.trim();    

      // establishing contact with Arduino
      if (firstContact == false) {
        if (arduinoData.equals("A")) {
          arduinoPort.clear();
          firstContact = true;
          arduinoPort.write("A");
          println("contact");
        }
      } else {
        if (arduinoData.indexOf("screen") == 0) {        
          String screenVal = arduinoData.substring(6);
          newScreenNo = int(screenVal);

          background(0); // clear previous images from the screen

          if (newScreenNo == -1) {
            newScreenNo = previousScreenNo;
          } else if (newScreenNo == 0) {
            showPromptToFinish = false; //reset flag
            t = 60; // reset the "automatic reset" timer
            timeFinishScreenSet = 0;
          } else if (newScreenNo == 46) {
            timeFinishScreenSet = millis();
          }

          image(storyImages[newScreenNo], 0, 0);  

          previousScreenNo = currentScreenNo;
          currentScreenNo = newScreenNo;
        } else if (arduinoData.indexOf("vol") == 0) {
          String volumeVal = arduinoData.substring(3);
          float volume = float(volumeVal);

          storyMovies[0].volume(volume);

          storyMovies[1].volume(volume);

          storyMovies[2].volume(volume);
        } else if (arduinoData.indexOf("play") == 0) {
          playFlag=true;
          if (currentScreenNo == 7) {
            storyMovies[0].play();
          } else if (currentScreenNo == 23) {
            storyMovies[1].play();
          } else if (currentScreenNo == 35) {

            storyMovies[2].play();
          }
        } else if (arduinoData.indexOf("pause") == 0) {
          if (newScreenNo == 7) {
            storyMovies[0].pause();
          } else if (newScreenNo == 23) {
            storyMovies[1].pause();
          } else if (newScreenNo == 35) {
            storyMovies[2].pause();
          }
        } else if (arduinoData.indexOf("stop") == 0 || arduinoData.indexOf("stop-flag") == 0) {
          playFlag=false;  
          if (currentScreenNo == 7) {
            storyMovies[0].stop();
          } else if (currentScreenNo == 23) {
            storyMovies[1].stop();
          } else if (currentScreenNo == 35) {
            storyMovies[2].stop();
          }
        } else if (arduinoData.indexOf("allTopicsVisited") == 0) {

          showPromptToFinish = true;
        } else if (arduinoData.indexOf("Experiment") == 0) {
          // save current table & clear for another experiment
          String fileName = str(year()) + str(month()) + str(day()) + str(hour()) + str(minute())+ str(second()) + ".csv";
          saveTable(dataLogTable, fileName);
          dataLogTable.clearRows();
        }
        // Record Arduino Serial output
        TableRow newRow = dataLogTable.addRow();
        //record time stamp
        newRow.setInt("year", year());
        newRow.setInt("month", month());
        newRow.setInt("day", day());
        newRow.setInt("hour", hour());
        newRow.setInt("minute", minute());
        newRow.setInt("second", second());
        newRow.setString("interaction", arduinoData);
      }
    }
  }
}


// Called every time a new frame is available to read
void movieEvent(Movie m) {
  m.read();
}