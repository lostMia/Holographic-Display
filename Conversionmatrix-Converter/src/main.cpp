#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <utility>

using namespace std;

const int ANGLES_PER_ROTATION = 360;
const int LEDS_PER_STRIP = 64;

// Function prototypes
vector<vector<pair<int, int>>> create_conversion_matrix(int center_x, int center_y);
void print_conversion_matrix_pretty(vector<vector<pair<int, int>>> *conversion_matrix);
void print_conversion_matrix_array(vector<vector<pair<int, int>>> *conversion_matrix);

//  - - - - - - - - - - Code - - - - - - - - - -

int main() 
{
  int center_x = LEDS_PER_STRIP / 2, center_y = LEDS_PER_STRIP / 2;

  auto conversion_matrix = create_conversion_matrix(center_x, center_y);

  // print_conversion_matrix_pretty(&conversion_matrix);
  print_conversion_matrix_array(&conversion_matrix);

  return 0;
}

void print_conversion_matrix_array(vector<vector<pair<int, int>>> *conversion_matrix) 
{
  // [angle][led_index] = [x][y]
  
  cout << "const Coordinates conversion_matrix[ANGLES_PER_ROTATION][LEDS_PER_STRIP] = \n{\n";

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


void print_conversion_matrix_pretty(vector<vector<pair<int, int>>> *conversion_matrix) 
{
  for (uint16_t angle = 0; angle < ANGLES_PER_ROTATION; angle++) 
  {
    cout << "Angle " << angle << " Degrees:\n";

    for (int led_index = 0; led_index < LEDS_PER_STRIP; led_index++) 
    {
      pair<int, int> coordinates = (*conversion_matrix)[angle][led_index];
      cout << " LED: " << led_index << " -> (x, y): (" << coordinates.first << ", " << coordinates.second << ")";
    }

    cout << "\n - - - - - - - - - - - - - - - -\n";
  }
}


// Define a 2D array to store (x, y) coordinates for each angle and led index
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
