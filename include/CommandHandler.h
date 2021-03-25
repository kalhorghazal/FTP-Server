#ifndef COMMANDHANDLER_H_
#define COMMANDHANDLER_H_

#include "UserManager.h"
#include "Utilities.h"
#include "Configuration.h"
#include "User.h"
#include "UserIdentityInfo.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>

#define COMMAND 0
#define ARG1 1
#define ARG2 2

#define USER_COMMAND "user"
#define PASS_COMMAND "pass"
#define PWD_COMMAND "pwd"
#define MKD_COMMAND "mkd"
#define DELE_COMMAND "dele"
#define LS_COMMAND "ls"
#define CWD_COMMAND "cwd"
#define RENAME_COMMAND "rename"
#define RETR_COMMAND "retr"
#define HELP_COMMAND "help"
#define QUIT_COMMAND "quit"

#define DELE_DIRECTORY_OPTION "-d"
#define DELE_FILE_OPTION "-f"

#define USER_DESCRIPTION "USER [name], Its argument is used to specify the user's string. It is used for user authentication.\n"
#define PASS_DESCRIPTION "PASS [password], Its argument is used to specify the user's password. It is used for user authentication.\n"
#define PWD_DESCRIPTION "PWD, It is used to print the name of the current working directory.\n"
#define MKD_DESCRIPTION "MKD [path], Its argument is used to specify the directory's path. It is usede to create a new directory.\n"
#define DELE_DESCRIPTION "DELE [flag] [path], Its argument is used to specify the file/directory's path. It flag is used to specify whether a file (-f) or a directory (-d) will be removed. It is usede to remove a file or directory.\n"
#define LS_DESCRIPTION "LS. It is used to print the list of files/directories in the current working directory.\n"
#define CWD_DESCRIPTION "CWD [path], Its argument is used to specify the directory's path. It is used to change the current working directory.\n"
#define RENAME_DESCRIPTION "RENAME [from] [to], Its arguments are used to specify the old and new file's name. It is used to change A file's name.\n"
#define RETR_DESCRIPTION "RETR [name], Its argument is used to specify the file's name. It is used to download a file.\n"
#define HELP_DESCRIPTION "HELP, It is used to display information about builtin commands.\n"
#define QUIT_DESCRIPTION "QUIT, It is used to sign out from the server.\n"

class CommandHandler {
public:
    CommandHandler(Configuration configuration);

    ~CommandHandler();

    UserManager* get_user_manager();

    std::vector<std::string> do_command(int user_socket, char* command);

    std::vector<std::string> handle_username(std::string username, User* user);
    std::vector<std::string> handle_password(std::string password, User* user);
    std::vector<std::string> handle_get_current_directory(User* user);
    std::vector<std::string> handle_create_new_directory(std::string dir_path, User* user);
    std::vector<std::string> handle_delete_directory(std::string dir_path, User* user);
    std::vector<std::string> handle_delete_file(std::string file_path, User* user);
    std::vector<std::string> handle_get_list_of_files(User* user);
    std::vector<std::string> handle_change_working_directory(std::string dir_path, User* user);
    std::vector<std::string> handle_rename_file(std::string old_name, std::string new_name, User* user);
    std::vector<std::string> handle_download_file(std::string);
    std::vector<std::string> handle_help();
    std::vector<std::string> handle_logout(User* user);

private:
    //User* logged_in;
    UserManager* user_manager;
};

#endif
