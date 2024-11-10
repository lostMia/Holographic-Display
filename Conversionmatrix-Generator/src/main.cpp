#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>

using namespace std;


//  - - - - - - - - - - Constants - - - - - - - - - -

const int ANGLES_PER_ROTATION = 360;
const int LEDS_PER_STRIP = 64;

//  - - - - - - - - - - Function Declarations - - - - - - - - - -

vector<vector<pair<int, int>>> create_conversion_matrix(int center_x, int center_y);
void print_conversion_matrix_pretty(vector<vector<pair<int, int>>> *conversion_matrix);
void print_conversion_matrix_array(vector<vector<pair<int, int>>> *conversion_matrix);
void print_shown_coordinates(vector<vector<pair<int, int>>> *conversion_matrix);

//  - - - - - - - - - - Function Definitons - - - - - - - - - -

int main() 
{
  int center_x = LEDS_PER_STRIP, center_y = LEDS_PER_STRIP;

  auto conversion_matrix = create_conversion_matrix(center_x, center_y);

  // print_conversion_matrix_pretty(&conversion_matrix);
  print_conversion_matrix_array(&conversion_matrix);
  // print_shown_coordinates(&conversion_matrix);

  return 0;
}

// Prints out the actual conversionm matrix usable in cpp.
void print_conversion_matrix_array(vector<vector<pair<int, int>>> *conversion_matrix) 
{
  cout << "const Coordinates conversion_matrix[ANGLES_PER_ROTATION][LEDS_PER_STRIP] PROGMEM = \n{\n";

  for (uint16_t angle = 0; angle < ANGLES_PER_ROTATION; angle++) 
  {
    cout << "  { ";
    for (int led_index = 0; led_index < LEDS_PER_STRIP; led_index++) 
    {
      pair<int, int> coordinates = (*conversion_matrix)[angle][led_index];
      cout << "{" << coordinates.first << ", " << coordinates.second << "}, ";
    }

    cout << " },\n";
  }

  cout << "};";
}

// Prints out values in a human readable way. Useful for debugging.
void print_conversion_matrix_pretty(vector<vector<pair<int, int>>> *conversion_matrix) 
{
  for (uint16_t angle = 0; angle < ANGLES_PER_ROTATION; angle++) 
  {
    cout << "Angle " << angle << " Degrees:\n";

    for (int led_index = 60; led_index < LEDS_PER_STRIP; led_index++) 
    {
      pair<int, int> coordinates = (*conversion_matrix)[angle][led_index];
      cout << " LED: " << led_index << " -> (x, y): (" << coordinates.first << ", " << coordinates.second << ")";
    }

    cout << "\n - - - - - - - - - - - - - - - -\n";
  }
}

// Prints out the affected coordinates in a 2D Array. Useful for showing the cardesian coordinates
// that certain LEDs will display.
void print_shown_coordinates(vector<vector<pair<int, int>>> *conversion_matrix) 
{
  char coordinate_field[LEDS_PER_STRIP * 2][LEDS_PER_STRIP * 2];

  memset(&coordinate_field, ' ', LEDS_PER_STRIP * 2 * LEDS_PER_STRIP * 2);

  // Shows *all* the affected coordinates.
  // for (uint16_t angle = 0; angle < ANGLES_PER_ROTATION; angle++) 
  // {
  //   for (int led_index = 0; led_index < LEDS_PER_STRIP; led_index++) 
  //   {
  //     pair<int, int> coordinates = (*conversion_matrix)[angle][led_index];
  //     coordinate_field[coordinates.first][coordinates.second] = '#';
  //   }
  // }

  // Shows only coordinates affected by a certain LED.
  for (uint16_t angle = 0; angle < ANGLES_PER_ROTATION; angle++) 
  {
    // LED 50 for example.
    uint16_t led_index = 50;

    pair<int, int> coordinates = (*conversion_matrix)[angle][led_index];
    coordinate_field[coordinates.first][coordinates.second] = '#';
  }

  // Prints out the full coordinate system.
  for (uint16_t x = 0; x < LEDS_PER_STRIP * 2; x++)
  {
    for (uint16_t y = 0; y < LEDS_PER_STRIP * 2; y++)
    {
      // Print every character twice, because every character has about a 2:1 ratio.
      cout << coordinate_field[x][y] << coordinate_field[x][y];
    }
    cout << "\n";
  }
}

// Define a 2D array to store (x, y) coordinates for each angle and led index.
vector<vector<pair<int, int>>> create_conversion_matrix(int center_x, int center_y)
{
  vector<vector<pair<int, int>>> conversion_matrix(
    ANGLES_PER_ROTATION, vector<pair<int, int>>(LEDS_PER_STRIP, {0, 0}));

  for (int angle = 0; angle < ANGLES_PER_ROTATION; angle++) 
  {
    double theta = angle * M_PI / 180.0;

    for (int led_index = 0; led_index < LEDS_PER_STRIP; led_index++) 
    {
      int x = static_cast<int>(round(center_x + led_index * cos(theta)));
      int y = static_cast<int>(round(center_y + led_index * sin(theta)));
      
      if (x >= 0 && x < 128 && y >= 0 && y < 128)
      {
        conversion_matrix[angle][led_index] = make_pair(x, y);
      } 
      else
      {
        conversion_matrix[angle][led_index] = make_pair(-1, -1);
      }
    }
  }
  
  return conversion_matrix;
}
