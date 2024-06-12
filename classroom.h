#include <string>
#define MAX_QUEUE_SIZE 10 // Define a maximum size for the queue

// Define the Student class
class Student {
public:
    String name;
    String student_number;
    String question;
    int ticket_number;

    Student(const String& name = "", const String& student_number = "", const String& question = "", const int& ticket_number = 0)
        : name(name), student_number(student_number), question(question) , ticket_number(ticket_number) {}
};

// Define the Classroom class
class Classroom {
public:
    String current_name;
    String current_student_number;
    String current_question;
    int current_ticket_number;
    bool away_from_desk;
    Student queue[MAX_QUEUE_SIZE];
    int queue_size;

    // Constructor with default parameters
    Classroom(const String& current_name = "", const String& current_student_number = "", const String& current_question = "", const int& current_ticket_number = 0, bool away_from_desk = false)
        : current_name(current_name), current_student_number(current_student_number), current_question(current_question), current_ticket_number(current_ticket_number), away_from_desk(away_from_desk), queue_size(0)  {}

    bool addStudentToQueue(const Student& student) {
        if (queue_size < MAX_QUEUE_SIZE) {
            queue[queue_size++] = student;            
            return true;
        } else {
            Serial.println("Queue is full!");
            return false;
        }
    }

    void setCurrentStudentFromQueue() {
        if (queue_size > 0) {
            current_name = queue[0].name;
            current_student_number = queue[0].student_number;
            current_ticket_number = queue[0].ticket_number;
            current_question = queue[0].question;            
        } else {
            Serial.println("Queue is empty!");
        }
    }

    bool setCurrentStudentByNumber(const String& student_number) {
      if (student_number == "") {
        removeCurrentStudent();
        return true;
      }
        for (int i = 0; i < queue_size; ++i) {
            if (queue[i].student_number == student_number) {
                current_name = queue[i].name;
                current_student_number = queue[i].student_number;
                current_ticket_number = queue[i].ticket_number;
                current_question = queue[i].question;
                return true;
            }
        }
        Serial.println("Student not found!");
        return false;
    }

    bool removeStudentByNumber(const String& student_number) {        
        for (int i = 0; i < queue_size; ++i) {
            if (queue[i].student_number == student_number) {
                Serial.println("removing student...");
                deleteElement(queue, queue_size, i);                
                return true;
            }
        }
        Serial.println("Student not found and not removed!");
        return false;
    }

    bool removeCurrentStudentByNumber(const String& student_number) {                
        if (current_student_number == student_number) {
            Serial.println("Removing currnet student...");
            current_name = "";
            current_student_number = "";
            current_ticket_number = 0;
            current_question = "";
            return true;
        }
        Serial.println("Student not found and current not changed!");
        return false;
    }

    void removeCurrentStudent() {                
        Serial.println("Removing currnet student...");
        current_name = "";
        current_student_number = "";
        current_ticket_number = 0;
        current_question = "";
    }
    
    void deleteElement(Student array[], int &size, int index) {
        if (index < 0 || index >= size) {
          // Index out of bounds
          Serial.println("Index out of bounds");
          return;
        }

        for (int i = index; i < size - 1; i++) {
          array[i] = array[i + 1];
        }

        // Decrease size
        size--;
    }

    void toggle_away() {
      away_from_desk = !away_from_desk;
    }
        
    void printClassroomInfo() const {
        Serial.println("Current Name: " + current_name);
        Serial.println("Current Student Number: " + current_student_number);
        Serial.println("Current Question: " + current_question);
        Serial.println("Current Ticket Number: " + current_ticket_number);
        Serial.println("Away From Desk: " + String(away_from_desk ? "Yes" : "No"));

        Serial.println("Queue:");
        for (int i = 0; i < queue_size; ++i) {
            Serial.println("{");
            Serial.println("  Name: " + queue[i].name);
            Serial.println("  Student Number: " + queue[i].student_number);
            Serial.println("  Ticket Number: " + queue[i].ticket_number);
            Serial.println("  Question: " + queue[i].question);
            Serial.println("}");
        }
    }
};
