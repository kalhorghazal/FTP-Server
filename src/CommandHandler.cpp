#include "CommandHandler.h"

using namespace std;

CommandHandler::CommandHandler(Configuration configuration, Logger* logger) {
    user_manager = new UserManager(configuration);
    this->logger = logger;
}

CommandHandler::~CommandHandler() {
    delete user_manager;
}

UserManager* CommandHandler::get_user_manager() {
    return user_manager;
}

vector<string> CommandHandler::do_command(int user_socket, char* command) {
    vector<string> command_parts = parse_command(command);

    User* user = user_manager->get_user_by_socket(user_socket);
    if (user == nullptr)
        return {GENERAL_ERROR, EMPTY};

    if (command_parts[COMMAND] == USER_COMMAND)
        return handle_username(command_parts[ARG1], user);

    else if (command_parts[COMMAND] == PASS_COMMAND)
        return handle_password(command_parts[ARG1], user);

    else if (user->get_state() != User::State::LOGGED_IN)
        return {NOT_AUTHORIZED, EMPTY};

    else if (command_parts[COMMAND] == PWD_COMMAND)
        return handle_get_current_directory(user);

    else if (command_parts[COMMAND] == MKD_COMMAND)
        return handle_create_new_directory(command_parts[ARG1], user);

    else if (command_parts[COMMAND] == DELE_COMMAND && command_parts[ARG1] == DELE_DIRECTORY_OPTION)
        return handle_delete_directory(command_parts[ARG2], user);

    else if (command_parts[COMMAND] == DELE_COMMAND && command_parts[ARG1] == DELE_FILE_OPTION)
        return handle_delete_file(command_parts[ARG2], user);

    else if (command_parts[COMMAND] == LS_COMMAND) 
        return handle_get_list_of_files(user);

    else if (command_parts[COMMAND] == CWD_COMMAND)
        return handle_change_working_directory(((command_parts.size() >= 2) ? command_parts[ARG1] : ""), user);

    else if (command_parts[COMMAND] == RENAME_COMMAND)
        return handle_rename_file(command_parts[ARG1], command_parts[ARG2], user);

    else if (command_parts[COMMAND] == RETR_COMMAND)
        return handle_download_file(command_parts[ARG1], user);

    else if (command_parts[COMMAND] == HELP_COMMAND)
        return handle_help();

    else if (command_parts[COMMAND] == QUIT_COMMAND)
        return handle_logout(user);

    else
        return {GENERAL_ERROR, EMPTY};
}

vector<std::string> CommandHandler::handle_username(string username, User* user) {
    if(user->get_state() != User::State::WAITING_FOR_USERNAME)
        return {BAD_SEQUENCE, EMPTY};

    UserIdentityInfo* user_identity_info = user_manager->get_user_info_by_username(username);

    if (user_identity_info == nullptr)
        return {INVALID_USER_PASS, EMPTY};
    
    user->set_state(User::State::WAITING_FOR_PASSWORD);
    user->set_user_identity_info(user_identity_info);

    return {USERNAME_ACCEPTED, EMPTY};
}

vector<std::string> CommandHandler::handle_password(string password, User* user) {
    if(user->get_state() != User::State::WAITING_FOR_PASSWORD)
        return {BAD_SEQUENCE, EMPTY};

    if (user->get_user_identity_info()->get_password() != password)
        return {INVALID_USER_PASS, EMPTY};

    user->set_state(User::State::LOGGED_IN);

    logger->log(user->get_username() + COLON + "logged in.");

    return {SUCCESSFUL_LOGIN, EMPTY};
}

vector<string> CommandHandler::handle_get_current_directory(User* user) {
    string bash_command = "realpath " + user->get_current_directory() + " > file.txt";
    int status = system(bash_command.c_str());
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};

    string result = read_file_to_string("file.txt");
    status = system("rm file.txt");
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};

    return {"257: " + result, EMPTY};
}

vector<string> CommandHandler::handle_create_new_directory(string dir_path, User* user) {
    string bash_command = "mkdir " + user->get_current_directory() + dir_path;
    int status = system(bash_command.c_str());
    if (status == 0) {
        string message = COLON + dir_path + " created.";
        logger->log(user->get_username() + message);
        return {CREATE_CODE + message, EMPTY};
    }
    return {GENERAL_ERROR, EMPTY};
}

vector<string> CommandHandler::handle_delete_directory(string dir_path, User* user) {
    string bash_command = "rm -r " + user->get_current_directory() + dir_path;
    int status = system(bash_command.c_str());
    if (status == 0) {
        string message = COLON + dir_path + " deleted.";
        logger->log(user->get_username() + message);
        return {DELETE_CODE + message, EMPTY};
    }
    return {GENERAL_ERROR, EMPTY};
}

vector<string> CommandHandler::handle_delete_file(string file_name, User* user) {
    string bash_command = "rm " + user->get_current_directory() + file_name;
    int status = system(bash_command.c_str());
    if (status == 0) {
        string message = COLON + file_name + " deleted.";
        logger->log(user->get_username() + message);
        return {DELETE_CODE + message, EMPTY};
    }
    return {GENERAL_ERROR, EMPTY};
}

vector<string> CommandHandler::handle_get_list_of_files(User* user) {
    string bash_command = "ls " + user->get_current_directory() + " > file.txt";
    int status = system(bash_command.c_str());
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};

    string result = read_file_to_string("file.txt");
    status = system("rm file.txt");
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};

    erase_sub_str(result, "file.txt\n");
    
    return {LIST_TRANSFER_DONE, result};
}

std::vector<std::string> CommandHandler::handle_change_working_directory(string dir_path, User* user) {
    if(dir_path == "")
        user->set_current_directory("");
    else
        user->set_current_directory(user->get_current_directory() + dir_path + "/");

    return {SUCCESSFUL_CHANGE, EMPTY};
}

std::vector<std::string> CommandHandler::handle_rename_file(string old_name, string new_name, User* user) {
    string bash_command = "mv " + user->get_current_directory() + old_name + " " +
            user->get_current_directory() + new_name;
    int status = system(bash_command.c_str());
    if (status == 0)
        return {SUCCESSFUL_CHANGE, EMPTY};
    return {GENERAL_ERROR, EMPTY};
}

std::vector<std::string> CommandHandler::handle_download_file(string file_name, User* user) {
    string file_path = user->get_current_directory() + file_name;
    string size_command = "stat -c%s " + file_path + " > " + "size.txt";
    int status = system(size_command.c_str());
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};
    
    double file_size = read_file_to_double("size.txt");
    status = system("rm size.txt");
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};

    if (user -> is_able_to_download(file_size) == false)
        return {DOWNLOAD_LIMIT_SIZE, EMPTY};

    string bash_command = "cp " + file_path + " file.txt";
    status = system(bash_command.c_str());
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};

    string result = read_file_to_string("file.txt");
    status = system("rm file.txt");
    if (status != 0)
        return {GENERAL_ERROR, EMPTY};

    user->decrease_available_size(file_size);

    string message = COLON + file_name + " downloaded.";
    logger->log(user->get_username() + message);

    return {SUCCESSFUL_DOWNLOAD, result};
}

std::vector<std::string> CommandHandler::handle_help() {
    string info = "214\n";
    info += USER_DESCRIPTION; 
    info += PASS_DESCRIPTION;
    info += PWD_DESCRIPTION;
    info += MKD_DESCRIPTION;
    info += DELE_DESCRIPTION;
    info += LS_DESCRIPTION;
    info += CWD_DESCRIPTION;
    info += RENAME_DESCRIPTION;
    info += RETR_DESCRIPTION;
    info += HELP_DESCRIPTION;
    info += QUIT_DESCRIPTION;
    return {info, EMPTY};
}

vector<string> CommandHandler::handle_logout(User* user) {    
    if (user->get_state() != User::State::LOGGED_IN)
        return {GENERAL_ERROR, EMPTY};

    user->set_state(User::State::WAITING_FOR_USERNAME);

    logger->log(user->get_username() + COLON + "logged out.");

    return {SUCCESSFUL_QUIT, EMPTY};
}

string CommandHandler::get_username_by_socket(int user_socket) {
    User* user = user_manager->get_user_by_socket(user_socket);
    if (user == nullptr)
        return "Anonymous";
    else if (user->get_state() != User::State::LOGGED_IN)
        return "Anonymous";
    return user->get_username();
}
