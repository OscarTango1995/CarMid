#include <SPI.h>
#include <SD.h>
#include "sd.h"
#include "gps.h"
#include "average.h"

#define SD_CS 33   // Chip select pin (CS)
#define SD_MOSI 23 // Master Out Slave In (MOSI)
#define SD_MISO 19 // Master In Slave Out (MISO)
#define SD_SCK 18  // Serial Clock (SCK)

int lineCount = 0;
const char *averageFileName = "/average.csv";
const char *fuelFileName = "/fuel.csv";
const char *gpsFileName = "/gps.csv";
const char *obdFileName = "/obd.csv";
const char *lineCountFileName = "/line_count.txt";
const char *errorFileName = "/error.txt";

void initAverageFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(averageFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(averageFileName, FILE_WRITE);
    if (csvFile)
    {
      csvFile.println("Average,Distance Traveled,Fuel Used,Fuel Remaining,Range,Time");
      csvFile.close();
      Serial.println("Average CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to open Average CSV file for writing.");
    }
  }
  else
  {
    Serial.println("Average CSV file already exists. No action taken.");
  }
}

void initFuelFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(fuelFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(fuelFileName, FILE_WRITE);
    if (csvFile)
    {
      csvFile.println("FF,Distance");
      csvFile.close();
      Serial.println("Fuel CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to open Fuel CSV file for writing.");
    }
  }
  else
  {
    Serial.println("Fuel CSV file already exists. No action taken.");
  }
}

void initGPSFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(gpsFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(gpsFileName, FILE_WRITE);
    if (csvFile)
    {
      csvFile.print("Latitude");
      csvFile.print(",");
      csvFile.print("Longitude");
      csvFile.print(",");
      csvFile.println("Distance");
      csvFile.close();
      Serial.println("GPS CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to create GPS CSV file for writing.");
    }
  }
  else
  {
    Serial.println("GPS CSV file already exists. No action taken.");
  }
}

void initOBDFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(obdFileName))
  {
    // Create the CSV file
    File csvFile = SD.open(obdFileName, FILE_WRITE);
    if (csvFile)
    {
      csvFile.print("Load");
      csvFile.print(",");
      csvFile.print("Manifold");
      csvFile.print(",");
      csvFile.print("Throttle");
      csvFile.print(",");
      csvFile.print("Rpm");
      csvFile.print(",");
      csvFile.print("FuelFlow");
      csvFile.print(",");
      csvFile.println("Timestamp");
      csvFile.close();
      Serial.println("OBD CSV file created with headers.");
    }
    else
    {
      Serial.println("Failed to create OBD CSV file for writing.");
    }
  }
  else
  {
    Serial.println("OBD CSV file already exists. No action taken.");
  }
}
void initLineCountFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(lineCountFileName))
  {
    // Create the CSV file
    File lineCountFile = SD.open(lineCountFileName, FILE_WRITE);
    if (lineCountFile)
    {
      lineCountFile.println(lineCount);
      lineCountFile.close();
      Serial.println("line count file created");
    }
    else
    {
      Serial.println("Failed to open line count file for writing.");
    }
  }
  else
  {
    Serial.println("line count file already exists. No action taken.");
  }
}

void initErrorFile()
{
  // Check if the CSV file already exists
  if (!SD.exists(errorFileName))
  {
    // Create the CSV file
    File errorFile = SD.open(errorFileName, FILE_WRITE);
    if (errorFile)
    {
      errorFile.close();
      Serial.println("error  file created");
    }
    else
    {
      Serial.println("Failed to open error file for writing.");
    }
  }
  else
  {
    Serial.println("error file already exists. No action taken.");
  }
}

void initSDCard(bool firstTime)
{
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI, 4000000))
  {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  if (firstTime)
  {
    initAverageFile();
    initFuelFile();
    initLineCountFile();
    initGPSFile();
    initErrorFile();
    initOBDFile();
  }
}

bool writeAvgToCSV(Average avg)
{
  String timestamp = getCurrentTimeStamp();
  File file = SD.open(averageFileName, FILE_APPEND);
  if (file)
  {
    // Write the data in CSV format
    file.print(avg.average, 5);
    file.print(",");
    file.print(avg.distanceTraveled);
    file.print(",");
    file.print(avg.fuelUsed, 5);
    file.print(",");
    file.print(avg.fuelRemaining, 5);
    file.print(",");
    file.print(avg.dte, 5);
    file.print(",");
    file.println(timestamp);
    file.close();
    Serial.println("Average Data logged to CSV.");
    return true;
  }
  else
  {
    Serial.println("Failed to open average file for writing.");
    return false;
  }
}

void writeOBDToCSV(float engineLoad, uint8_t manifoldPressure, float throttlePosition, float rpm, float fuelFlow)
{
  String timestamp = getCurrentTimeStamp();
  File file = SD.open(obdFileName, FILE_APPEND);
  if (file)
  {
    // Write the data in CSV format
    file.print(engineLoad);
    file.print(",");
    file.print(manifoldPressure);
    file.print(",");
    file.print(throttlePosition, 5);
    file.print(",");
    file.print(rpm, 5);
    file.print(",");
    file.print(fuelFlow, 5);
    file.print(",");
    file.println(timestamp);
    file.close();
    Serial.println("OBD Data logged to CSV.");
  }
  else
  {
    Serial.println("Failed to open obd file for writing.");
  }
}

void writeGPSToCSV(double latitude, double longitude, double distance)
{
  String timestamp = getCurrentTimeStamp();
  File file = SD.open(gpsFileName, FILE_APPEND);
  if (file)
  {
    // Write the data in CSV format
    file.print(latitude, 6);
    file.print(",");
    file.print(longitude),6;
    file.print(",");
    file.println(distance,6);
    file.close();
    Serial.println("GPS Data logged to CSV.");
  }
}

// Function to write line count to the metadata file
void writeLineCount()
{
  lineCount = readLineCount();
  lineCount++;
  File lineCountFile = SD.open(lineCountFileName, FILE_WRITE);
  if (lineCountFile)
  {
    lineCountFile.println(lineCount);
    lineCountFile.close();
  }
}

void writeError(String resetReason)
{
  File file = SD.open(errorFileName, FILE_APPEND); // Open file in append mode
  if (file)
  {
    file.println("Reset reason: " + resetReason);
    file.close();
    Serial.println("Reset reason written to SD card.");
  }
  else
  {
    Serial.println("Failed to open error file on SD card.");
  }
}

void writeFuelDistanceToCSV(float fuelFlow, float distance)
{
  String timestamp = getCurrentTimeStamp();
  File file = SD.open(fuelFileName, FILE_APPEND);
  if (file)
  {
    // Write the data in CSV format
    file.print(fuelFlow, 5);
    file.print(",");
    file.println(distance, 5);
    file.close();
    Serial.println("Fuel Data logged to CSV.");
  }
  else
  {
    Serial.println("Failed to open fuel file for writing.");
  }
}

// Function to read line count from the metadata file
int readLineCount()
{
  initSDCard(false);
  delay(300);
  File lineCountFile = SD.open(lineCountFileName, FILE_READ);
  if (lineCountFile)
  {
    lineCount = lineCountFile.parseInt();
    lineCountFile.close();
    return lineCount;
  }
  else
  {
    Serial.println("failed to open line count file");
    return 0;
  }
}

Total readFuelDistanceCSV()
{
  initSDCard(false);
  delay(300);
  Total total;
  float totalFuel = 0.0;
  double totalDistance = 0.0;

  // Open the fuel.csv file for reading
  File fuelFile = SD.open(fuelFileName, FILE_READ);
  if (!fuelFile)
  {
    Serial.println("Failed to open fuel.csv for reading.");
  }

  // Read the fuel.csv file line by line
  while (fuelFile.available())
  {
    String line = fuelFile.readStringUntil('\n'); // Read until newline
    int separatorIndex = line.indexOf(',');       // Find the separator
    if (separatorIndex == -1)
      continue; // Skip if no separator

    // Extract fuel flow and distance values
    String fuelFlowStr = line.substring(0, separatorIndex);
    String distanceStr = line.substring(separatorIndex + 1);

    // Convert to float and double
    float fuelFlow = fuelFlowStr.toFloat();
    double distance = distanceStr.toDouble();

    // Accumulate total fuel and distance
    totalFuel += fuelFlow;
    totalDistance += distance;
  }

  // Close the fuel file
  fuelFile.close();
  total.totalDistance = totalDistance;
  total.totalFuel = totalFuel;
  return total;
}

GPS readLastCoordinates()
{
  initSDCard(false);
  delay(300);
  GPS gps;
  gps.latitude = 0.0;
  gps.longitude = 0.0;
  gps.distance = 0.0;

  // Open the gps.csv file for reading
  File gpsFile = SD.open(gpsFileName, FILE_READ);
  // if (gpsFile)
  // {
  //   String line = gpsFile.readStringUntil('\n'); // Read the first line (single entry)
  //   gpsFile.close();

  //   // Split the line by the comma to get latitude and longitude
  //   int commaIndex = line.indexOf(',');
  //   if (commaIndex > 0)
  //   {
  //     gps.latitude = line.substring(0, commaIndex).toDouble();   // Convert substring to double for latitude
  //     gps.longitude = line.substring(commaIndex + 1).toDouble(); // Convert substring to double for longitude
  //   }

  //   return gps;
  // }
  // else
  // {
  //   Serial.println("Failed to open the gps CSV file for reading.");
  //   return gps;
  // }
  String lastLine = "";
  while (gpsFile.available())
  {
    String line = gpsFile.readStringUntil('\n'); // Read each line
    if (line.length() > 0)
    {
      lastLine = line; // Store the last non-empty line
    }
  }

  gpsFile.close();

  if (lastLine.length() == 0)
  {
    Serial.println("No valid data found.");
    return gps; // Return empty avg if no valid data
  }
  // Now parse the last line into an Average struct
  int index = 0;
  String value;

  // Split by comma and extract fields
  value = lastLine.substring(0, lastLine.indexOf(','));
  gps.latitude = value.toDouble(); // First value is average
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  gps.longitude = value.toDouble(); // Second value is distance travelled
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  gps.distance = value.toDouble(); // Third value is fuel level
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);
  return gps;
}

Average readLastAverageData()
{
  initSDCard(false);
  delay(300);
  File file = SD.open(averageFileName, FILE_READ);

  Average avg;
  avg.average = 0;
  avg.distanceTraveled = 0;
  avg.fuelUsed = 0.0;
  avg.fuelRemaining = 0.0;
  avg.dte = 0.0;

  if (file.size() == 0)
  {
    Serial.println("File is empty.");
    return avg;
  }
  if (!file)
  {
    Serial.println("Failed to open file for reading.");
    return avg;
  }

  String lastLine = "";
  while (file.available())
  {
    String line = file.readStringUntil('\n'); // Read each line
    if (line.length() > 0)
    {
      lastLine = line; // Store the last non-empty line
    }
  }

  file.close();

  if (lastLine.length() == 0)
  {
    Serial.println("No valid data found.");
    return avg; // Return empty avg if no valid data
  }
  // Now parse the last line into an Average struct
  int index = 0;
  String value;

  // Split by comma and extract fields
  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.average = value.toFloat(); // First value is average
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.distanceTraveled = value.toDouble(); // Second value is distance travelled
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.fuelUsed = value.toFloat(); // Third value is fuel level
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.fuelRemaining = value.toFloat(); // fourth value is fuel Remaining
  lastLine = lastLine.substring(lastLine.indexOf(',') + 1);

  value = lastLine.substring(0, lastLine.indexOf(','));
  avg.dte = value.toFloat(); // Fourth value is dte

  return avg;
}

void resetAverageFile()
{
  if (SD.exists(averageFileName))
  {
    Serial.println("Average File exists, deleting now...");

    if (SD.remove(averageFileName))
    {
      Serial.println("Average File deleted successfully");
      initAverageFile();
    }
    else
    {
      Serial.println("Failed to delete average file");
    }
  }
}

void resetGPSFile()
{
  if (SD.exists(gpsFileName))
  {
    Serial.println("GPS File exists, deleting now...");

    if (SD.remove(gpsFileName))
    {
      Serial.println("GPS File deleted successfully");
      initGPSFile();
    }
    else
    {
      Serial.println("Failed to delete gps file");
    }
  }
}

void resetLineCount()
{
  Serial.println("Resetting line count");
  File lineCountFile = SD.open(lineCountFileName, FILE_WRITE);
  if (lineCountFile)
  {
    lineCountFile.println(0);
    lineCountFile.close();
  }
}

void resetFuelDistanceCSV()
{
  if (SD.exists(fuelFileName))
  {
    Serial.println("Fuel File exists, deleting now...");

    if (SD.remove(fuelFileName))
    {
      Serial.println("Fuel File deleted successfully");
      initFuelFile();
    }
    else
    {
      Serial.println("Failed to delete fuel file");
    }
  }
}

void deleteAllFiles()
{
  resetAverageFile();
  resetFuelDistanceCSV();
  resetLineCount();
  resetGPSFile();
  initSDCard(true);
}