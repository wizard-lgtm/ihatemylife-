#ifndef NOTE_H
#define NOTE_H
typedef struct {
    char* title; 
    char* content; // Markdown

} Note;

Note* create_note(char* title, char* content);
void free_note(Note*);
#endif