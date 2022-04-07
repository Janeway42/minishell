#include "minishell.h"


//testing function
t_cmd_block *create_cmd_block()
{
	t_cmd_block *first= malloc(sizeof (t_cmd_block));
	char *cmd_first[]={"ls","-la",NULL};
	first->cmd_args = cmd_first;
	first->inputfd = 0;
	first->outputfd = 1;
	first->redirs = NULL;
	first->cmdnbr = 1;
	first->next = NULL;
	return (first);
}

void	sig_handler(int sig_no)
{
	if (sig_no == SIGINT)
	{
		write(1, "\n", 1);
		rl_replace_line("", 0);
		rl_on_new_line();
		rl_redisplay();
	}
}

void	set_up_shell_terminal(t_data *data)
{
	if (!isatty(0))
		exit_on_error("Error :", 1);
	else
	{
		if (tcgetattr(0, &(data->old_term)) == -1)
			exit_on_error("Error :", 1);
		data->new_term = data->old_term;
//		data->new_term.c_lflag &= ~(0001000); //linux flag
		data->new_term.c_lflag &= ~(ECHOCTL); //mac flag
		if (tcsetattr(0, TCSANOW, &(data->new_term)) == -1)
			exit_on_error("Error :", 1);
	}
}

void	exec_command(t_cmd_block *cmd_list, t_data *data)
{
	char *path;
//do the redirections
//do the pipe redirections first
	dup2(STDIN_FILENO)
//do the other redictions after
//set to the old terminal or not if it has been reset right after the input
	//tcsetattr(0, TCSANOW, &data->old_term);
//check if it is a builtin command
	if (is_it_builtin(cmd_list->cmd_args[0]))
		execute_builtin(&data->envp_list, &cmd_list->cmd_args[1], data->last_exit_code);
//if not check if the command exist if not give error
//else execute the command
	else
	{
		path = validate_and_locate_cmd(cmd_list->cmd_args[0],data->envp_list);
printf("path is %s\n",path);
		if (path == NULL)
		{
			write(2, cmd_list->cmd_args[0], ft_strlen(cmd_list->cmd_args[0]));
			write(2, ": command not found\n", 20);
			exit(127);
		}
		if (execve(path, cmd_list->cmd_args, data->envp_list))
			exit_on_error(cmd_list->cmd_args[0], 1);
	}
}

//	function to count the amount of commands in the command list

int	count_commands(t_cmd_block *cmd_list)
{
	int count;

	count = 0;
	while (cmd_list != NULL)
	{
		count++;
		cmd_list = cmd_list->next;
	}
	return (count);
}

//	function that process the command block

void	process_cmds(t_cmd_block *cmd_list, t_data *data)
{
	int pid;
	int wstatus;
	int amount_commands;
	int	cmdnbr;

	amount_commands = count_commands(cmd_list);
	//create all the heredoc files
	cmdnbr = 1;
	while (cmdnbr <= amount_commands)
	{
		//if there is still another command thus pipe is necessary
		if (cmdnbr < amount_commands)
		{
			//if not the first command copy read end fd of the previous outpipe to inpipe_fd
			if (cmdnbr != 1)
				data->inpipe_fd = data->outpipe_fds[0];
			if (pipe(data->outpipe_fds) == -1)
				exit_on_error("Error :", 1);
		}

		//if it is the only command and it is builtin
		if (amount_command == 1 && is_it_builtin(cmd_list->cmd_args[0]))
		{	
			//do the redirections
			//don't fork and execute the builtin function
			data->last_exit_code = execute_builtin(&data->envp_list
				, cmd_list->cmd_args, data->last_exit_code);
		}
		else
		{
			pid = fork();
			if (pid < 0)
				exit_on_error("Error :", 1);
			else if (pid == 0)
				exec_command(cmd_list, data);
			else
			{
				//only close the inputfd when not first commmand 
				//because there is no input pipe for the first command.
				if (cmdnbr != 1)
					close(cmd_list->inputfd);
				//only close the output write side of pipe if not last command
				//because there is outputpipe for the last command
				if (cmdnbr != amount_commands)
  					close(outpipefd[1]);
			}
		}
		cmd_list = cmd_list->next;
		cmdnbr++;
	}

//temporary wait for all children processses
	cmdnbr = 1;
	while (cmdnbr <= amount_commands)
	{
		wait(&wstatus);
		cmdnbr++;
	}
}


void	init_minishell(int argc, char **argv, char **envp, t_data *data)
{
	(void) argc;
	(void) argv;
	signal(SIGINT, sig_handler);
	signal(SIGQUIT, SIG_IGN);
	data->envp_list = copy_envp(envp);
	set_variable(&(data->envp_list),"OLDPWD=");
	set_up_shell_terminal(data);
}

int	main(int argc, char **argv, char **envp)
{
	t_data		data;
	t_cmd_block	*cmd_list;
	char		*input;

	init_minishell(argc, argv, envp, &data);	
	cmd_list = create_cmd_block();
	while (1)
	{
		if (tcsetattr(0, TCSANOW, &(data.new_term)) == -1) //reset the terminal to allow echoctl
			exit_on_error("Error :", 1);
		input = readline(PROMPT);
		if (tcsetattr(0, TCSANOW, &(data.old_term)) == -1) //reset the terminal to allow echoctl
			exit_on_error("Error :", 1);
		if (input == NULL)
		{
			//maybe free all the things
			write(2, "exit\n", 5);
			exit(0);
		}
		if (input[0] != '\0')
			add_history(input);
//process the input into command blocks
//--------------------------------------------------------------------
if (input[0] != '\0')
{
	cmd_list->cmd_args = ft_split(input,' ');
	int i = 0;
	while (cmd_list->cmd_args[i] != NULL)
	{
		printf("%s\n", cmd_list->cmd_args[i]);
		i++;
	}

//--------------------------------------------------------------------
		process_cmds(cmd_list, &data);

		//don't forget to free the cmd_list
free_string_array(cmd_list->cmd_args);
}
		free(input);
//set the exit code in environment variable `?` to the exitcode
	}
	return (0);
}