/* Project: MSc project, Human-Computer Interaction programme, University College London */

ArrayList<Table> tables = new ArrayList<Table>();
Table resultsTable; 

void setup() {
  noLoop();

  resultsTable= new Table(); 
  resultsTable.addColumn("ID");
  resultsTable.addColumn("Experiment No.");
  resultsTable.addColumn("Time spent");
  resultsTable.addColumn("Enabled buttons pressed");
  resultsTable.addColumn("Disabled buttons pressed");
  resultsTable.addColumn("Experience finished");
  resultsTable.addColumn("All topics visited");
  resultsTable.addColumn("Initial opinion on bats");
  resultsTable.addColumn("Leart something new");
  resultsTable.addColumn("Opinion change");
  resultsTable.addColumn("Enjoyable experience");
  resultsTable.addColumn("Easy to use");

  // Collect all file names from the data folder
  String path = sketchPath("data");
  String[] filenames = listFileNames(path);
  printArray(filenames);
  //println(filenames.length);


  for (int i=0; i<filenames.length; i++) { 
    Table val = loadTable(filenames[i], "header");
    tables.add(val);
  }

  for (int i=0; i<tables.size(); i++) {
    Table table = tables.get(i);

    int enabledButtonsCount = 0;
    int disabledButtonsCount = 0;
    boolean sliderCountFlag = true;
    int timeStart = 0;
    int timeEnd = 0;
    String quizScreenNo = "";

    TableRow newRow = resultsTable.addRow(); // create a new row for each experiment in the results table
    newRow.setInt("ID", i);

    // write experiment number
    newRow.setString("Experiment No.", table.getString(0, "interaction"));

    // skip "screen 0" (j==1) - this is the default screen before the experience is initiated by the user
    for (int j=2; j<table.getRowCount(); j++) {

      String val = table.getString(j, "interaction");

      if (j==2) {
        int hourStart = table.getInt(j, "hour") * 3600; 
        int minuteStart = table.getInt(j, "minute") * 60; 
        int secondStart = table.getInt(j, "second"); 
        timeStart = hourStart + minuteStart + secondStart;
      }


      // count enabled button clicks
      if (val.indexOf("screen")==0 || val.indexOf("play")==0) {

        enabledButtonsCount++;
      } else if (val.indexOf("Slider")==0 && sliderCountFlag == true) {
        enabledButtonsCount++;
        sliderCountFlag = false;
      } else if (val.indexOf("Disabled")==0) {
        disabledButtonsCount++;
      } else if (val.indexOf("allTopicsVisited")==0) {
        newRow.setString("All topics visited", "yes");
      }

      if (val.equals("screen1")==true || val.equals("screen42")==true || val.equals("screen43")==true || val.equals("screen44")==true || val.equals("screen45")==true) {
        quizScreenNo = val;
      }

      if (quizScreenNo.equals("screen1")==true) {
        if (val.equals("EnabledButtonClicked: A")==true || val.equals("EnabledButtonClicked: B")==true || val.equals("EnabledButtonClicked: C")==true || val.indexOf("EnabledButtonClicked: D")==0 || val.indexOf("EnabledButtonClicked: E")==0) {
          newRow.setString("Initial opinion on bats", val.substring(22));
          quizScreenNo = ""; // reset variable
        }
      } else if (quizScreenNo.equals("screen42")==true) {
        if (val.indexOf("EnabledButtonClicked: A")==0 || val.indexOf("EnabledButtonClicked: B")==0 || val.indexOf("EnabledButtonClicked: C")==0 || val.indexOf("EnabledButtonClicked: D")==0 || val.indexOf("EnabledButtonClicked: E")==0) {
          newRow.setString("Leart something new", val.substring(22));
          quizScreenNo = ""; // reset variable
        }
      } else if (quizScreenNo.equals("screen43")==true) {
        if (val.indexOf("EnabledButtonClicked: A")==0 || val.indexOf("EnabledButtonClicked: B")==0 || val.indexOf("EnabledButtonClicked: C")==0 || val.indexOf("EnabledButtonClicked: D")==0 || val.indexOf("EnabledButtonClicked: E")==0) {
          newRow.setString("Opinion change", val.substring(22));
          quizScreenNo = ""; // reset variable
        }
      } else if (quizScreenNo.equals("screen44")==true) {
        if (val.indexOf("EnabledButtonClicked: A")==0 || val.indexOf("EnabledButtonClicked: B")==0 || val.indexOf("EnabledButtonClicked: C")==0 || val.indexOf("EnabledButtonClicked: D")==0 || val.indexOf("EnabledButtonClicked: E")==0) {
          newRow.setString("Enjoyable experience", val.substring(22));
          quizScreenNo = ""; // reset variable
        }
      } else if (quizScreenNo.equals("screen45")==true) {
        if (val.indexOf("EnabledButtonClicked: A")==0 || val.indexOf("EnabledButtonClicked: B")==0 || val.indexOf("EnabledButtonClicked: C")==0 || val.indexOf("EnabledButtonClicked: D")==0 || val.indexOf("EnabledButtonClicked: E")==0) {
          newRow.setString("Easy to use", val.substring(22));
          quizScreenNo = ""; // reset variable
        }
      }

      if (val.indexOf("screen46")==0 || j==(table.getRowCount() - 1)) {
        newRow.setInt("Enabled buttons pressed", enabledButtonsCount);
        newRow.setInt("Disabled buttons pressed", disabledButtonsCount);

        int hourEnd = table.getInt(j, "hour") * 3600; 
        int minuteEnd = table.getInt(j, "minute") * 60; 
        int secondEnd = table.getInt(j, "second"); 
        timeEnd = hourEnd + minuteEnd + secondEnd;

        newRow.setInt("Time spent", timeEnd - timeStart);

        if (val.indexOf("screen46")==0) {
          newRow.setString("Experience finished", "yes");
        } else if (resultsTable.getString(i, "Experience finished") == null) { 
          newRow.setString("Experience finished", "no");
        }

        table.clearRows();
        break;
      }
    }
  }
  saveTable(resultsTable, int(random(100)) + "resultsTable.csv"); 
  exit();
}

void draw() {
}

// This function returns all the files in a directory as an array of Strings  
String[] listFileNames(String dir) {
  File file = new File(dir);
  if (file.isDirectory()) {
    String names[] = file.list();
    return names;
  } else {
    // If it is not a directory
    return null;
  }
}