# Project 2: Command Line Shell

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-2.html

Mushahid Hassan  
CS 326, Section 1  
Professor Malensek  
Project 2  
  
This project was although lengthy, but it was extremely fun. I had a great experience implementing all the great shell  
features in my shell.   
The name of my shell is "cash" = "currency acquiring shell"  
  
I broke my work up in files.  
1) built_in.c: This file takes cares of the built in features of my shell.    
 - is_file function checks to see if the given path is a file or not.  
 - change_directory function changes the current directory to the given target directory.    
 - comment_check function checks to see the given line contains any comments.  
 - set_env function sets up a custom environment variable to the given name and value.  
   
2) history.c: This file takes cares of add, printing, searching the history database for my shell.  
 - starts_with function checks if the given token starts with an exclamation point.  
 - is_numeric is a function for checking whether the given string is numeric.    
 - append is a helper function that appends a character to a string.  
 - smallest_index function finds the index that has the smallest command number, which is used for printing.  
 - add_history function adds the given line to the history array.
 - print_history function prints the last 100 commands in history array.  
 - double_exclamation function prints the last command that was entered by the user.  
 - prefix_search function performs the prefix search on the history array and returns the latest search   
   result that matches the prefix that is being searched.  
 - num_search function searches the history array and looks for the given command number.  
   
3) pipe.c: This file is for handling the piping commands.  
 - populate_strcut is a function that parses the tokens into commands and stores it into the cmds struct.  
   The values are read from the new_token array, an array of all tokens parsed, cleaned, and separated in order.  
 - execute_pipeline is a recursive function that executes the given struct of commands and stores to  
   either an output.  
     
4) tokenizer.c: This file is for cleaning, parsing, replacing the whole line received in the commands into  
                something that is easy to use and call.  
 - next_token function retrieves the next token from a string. If the token contains environmental variables  
   or quotes, this function parsing those and returns it as one token.  
 - add_token function adds the given token to the new_tok array which is used for storing all the  
   tokens in order parsed, cleaned, and expanded.  
 - print_token function prints the new_token array.  
   
5) user_info.c: This file is for finding the current username, hostname, and home directory.  
  
6) shell.c: This is the main functionality of my program. It integreates all the files, and functions within  
            those files, to perform the full functionality of my shell. The functionality consists of built in  
            functions such as "cd", "setenv", "jobs", and "exit". My shell also copes with history, which consists  
            of last 100 commands, which is integrated in shell.c. This file also takes care of signal handling such  
            as SIGINT and SIGCHLD. Comments check, redirection, and piping are also implemented in shell.c along  
            with fork.
 
