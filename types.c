#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
Note* create_note(char* title, char* content){
    Note *mynote = malloc(sizeof(Note));
    if(mynote == NULL){
        perror("Can't create note");
        exit(EXIT_FAILURE);
    }

    mynote->title= malloc(sizeof(char) * strlen(title) + 1); 
    mynote->content = malloc(sizeof(char) * strlen(content) + 1);

    strcpy(mynote->title, title);
    strcpy(mynote->content, content);

    return mynote;
}
void free_note(Note *note){
    free(note->title);
    free(note->content);
    free(note);
}
