/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgu <hgu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/04 20:07:20 by hyuim             #+#    #+#             */
/*   Updated: 2023/11/05 20:14:25 by hgu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

//TODO :: redirection error -> err flag on
//TODO :: 1234 -> correct exit status
//TODO :: signal int, quit -> exit_status update.
//TODO :: exit status -> using waitpid, set the last cmd's status
//TODO :: ctrl + c during here_doc
//TODO :: get return of wait and get signals
//TODO :: rm tempfiles in hrdc list




int	exit_status;

int	exec_builtin(char *cmd_name, char **cmd_argv, t_bundle *bundle)
{
	if (ft_strlen(cmd_name) == 4 && !ft_strcmp(cmd_name, "echo"))
		return (echo(cmd_argv));
	if (ft_strlen(cmd_name) == 2 && !ft_strcmp(cmd_name, "cd"))
		return (cd(bundle, cmd_argv));
	if (ft_strlen(cmd_name) == 3 && !ft_strcmp(cmd_name, "pwd"))
		return (pwd());
	if (ft_strlen(cmd_name) == 6 && !ft_strcmp(cmd_name, "export"))
		return (export(bundle, cmd_argv));
	if (ft_strlen(cmd_name) == 5 && !ft_strcmp(cmd_name, "unset"))
		return (unset(bundle, cmd_argv));
	if (ft_strlen(cmd_name) == 3 && !ft_strcmp(cmd_name, "env"))
		return (env(bundle));
	if (ft_strlen(cmd_name) == 4 && !ft_strcmp(cmd_name, "exit"))
		ft_exit(cmd_argv);
	return (1);
}

t_pipe *parsing(t_bundle *bundle, char *rl)
{
	t_token *token_head;
	t_token *temp;
	t_pipe *root;

	token_head = tokenize(rl);
	temp = syntax_analyze(token_head);
	if (!temp)
	{
		write(2, "syntax error\n", 13);
		free(rl);
		return (NULL);
	}
	token_head = expansion_main(token_head, bundle);
	root = make_tree(token_head, bundle);
	free_token(token_head);
	return (root);
}

void	leaks()
{
	system("leaks minishell");
}

void	free_hrdc_nodes(t_bundle *bundle)
{
	t_list *temp;
	t_list *temp_for_before_one;
	t_list_hrdc *temp_hrdc;
	t_list_hrdc *temp_for_before_hrdc_one;

	temp = bundle->hrdc_filename_list;
	while (temp)
	{
		unlink(temp->content);
		free(temp->content);
		temp->content = NULL;
		temp_for_before_one = temp;
		temp = temp->next;
		free(temp_for_before_one);
	}
	temp_hrdc = bundle->hrdc_cnt_list;
	while (temp_hrdc)
	{
		temp_for_before_hrdc_one = temp_hrdc;
		temp_hrdc = temp_hrdc->next;
		free(temp_for_before_hrdc_one);
	}
}

void	print_tree(t_pipe *node, t_bundle *bundle);

int	check_file_exist(t_redirect_s *redirect_s)
{
	if (redirect_s && redirect_s->redirect && redirect_s->redirect->filename)
	{
		//printf("%s\n", redirect_s->redirect->filename);
		if (access(redirect_s->redirect->filename, F_OK))
		{
			write(2, redirect_s->redirect->filename, ft_strlen(redirect_s->redirect->filename));
			write(2, ": No such file or directory\n", 28);
			return (-1);
		}
		//write(2, "here hey!\n", 10);
	}
	if (redirect_s->redirect_s)
		if (check_file_exist(redirect_s->redirect_s) == -1)
			return (-1);
	return (0);
}

int main(int argc, char *argv[], char *envp[])
{
	char		*inp;
	t_pipe		*root;
	t_bundle	bundle;
	int			idx;
	int			before_stdin;
	int			before_stdout;
	int			exit_result;
	int			finished_pid;

	(void)argc;
	(void)argv;
	//atexit(leaks);

	cp_envp(&bundle, envp);
	rl_catch_signals = 0;
	while (1)
	{
		init_bundle(&bundle);
		idx = -1;
		//write(2, "--------------------here1\n", 26);
		//printf("envp_len : %d\n", bundle.envp_len);
		printf("exit status : %d\n", exit_status);
		inp = readline("ϞϞ(๑⚈ ․̫ ⚈๑) > ");
		//sleep(1); //////////////////////
		//inp = readline(">>>>>>>>>>>>>> ");
		//write(2, "--------------------here2\n", 26);
		if (!inp)
		{
			write(1, "exit\n", 5);
			exit(0);
		}
		if (!*inp)
			continue ;
		add_history(inp);
		root = parsing(&bundle, inp);
		free(inp);
		print_tree(root, &bundle);
		if (!root)
			continue ;
		if (check_one_cmd_and_builtin(root) == 1)
		{
			//write(2, "here4\n", 6);
			if (root->cmd->redirect_s)
			{
				before_stdin = dup(STDIN_FILENO);
				before_stdout = dup(STDOUT_FILENO);
				if (pre_exec_here_doc(root, &bundle) == -1)
				{
					dup2(before_stdin, STDIN_FILENO);
					dup2(before_stdout, STDOUT_FILENO);
					continue ;
				}
				exec_redirect_s_recur(root->cmd->redirect_s, &bundle);
			}
			exit_status = exec_builtin(root->cmd->simple_cmd->cmd_path, root->cmd->simple_cmd->cmd_argv, &bundle);
			if (root->cmd->redirect_s)
			{
				dup2(before_stdin, STDIN_FILENO);
				dup2(before_stdout, STDOUT_FILENO);
			}
		}
		else
		{
			//write(2, "exec multiple cmd\n", 18);
			if (pre_exec_here_doc(root, &bundle) != -1)
			{
				//printf("step1\n");
				//if (root && root->cmd && root->cmd->redirect_s)
				//{
				//	printf("step2\n");
				//	if (check_file_exist(root->cmd->redirect_s) != -1)
				//	{
				//		printf("step3\n");
				//		//write(2, "herehere\n", 9);
				//		if (root && root->cmd && root->cmd->simple_cmd && root->cmd->simple_cmd->cmd_path)
				//		{
							//printf("step4\n");
				exec_recur(root, &bundle, -1, 0);
				//sleep(100);
				//printf("cmd cnt : %d\n", bundle.cmd_cnt);
				signal(SIGINT, sigint_handler_during_fork);
				signal(SIGQUIT, sigquit_handler_during_fork);
				while (++idx < bundle.cmd_cnt)
				{
					finished_pid = wait(&exit_result);
					if (finished_pid == bundle.last_pid)
					{
						if (WIFSIGNALED(exit_result))
							exit_status = 128 + WTERMSIG(exit_result);
						else
							exit_status = WEXITSTATUS(exit_result);
					}
					//printf("exit status : %d\n", exit_status);
				}
				if (bundle.cmd_cnt == 0)
				{
					finished_pid = wait(&exit_result);
					if (WIFSIGNALED(exit_result))
						exit_status = 128 + WTERMSIG(exit_result);
					else
						exit_status = WEXITSTATUS(exit_result);

				}
				//		}
				//	}
				//}

			}
		}
		free_hrdc_nodes(&bundle);
		free_tree(root);
	}
	//after one builtin, reset redirection

//	(void)argc;
//	(void)argv;
//	(void)envp;
//	t_bundle bundle;
//	t_pipe root;
//	t_cmd cmd;
//	t_redirect_s redirs;
//	t_redirect_s redirs2;
//	t_redirect red1;
//	//t_redirect red2;
//	t_simple_cmd sim_cmd;

//	t_pipe pipe2;
//	t_cmd cmd2;
//	t_simple_cmd smp_cmd2;

////////////////////////////////////////
//	init_bundle(&bundle, envp);
//	bundle.cmd_cnt = 2;
////////////////////////////////////////

//	root.cmd = &cmd;
//	root.pipe = &pipe2;
//	cmd.redirect_s = &redirs;
//	cmd.simple_cmd = &sim_cmd;
//	redirs.redirect = &red1;
//	redirs.redirect_s = NULL;
//	//redirs.redirect_s = &redirs2;
//	//redirs2.redirect = &red2;
//	redirs2.redirect_s = NULL;
//	red1.type = REDIR_LEFT;
//	red1.filename = "infile";
//	//red2.type = REDIR_RIGHT;
//	//red2.filename = "outfile";
//	sim_cmd.cmd_path = "sleep";
//	char *argv_custom[3] = {"sleep", "3", NULL};
//	sim_cmd.cmd_argv = argv_custom;
//	//char *test = readline("> ");
//	//(void)test;
//	//int	idx = -1;
//	//while (bundle.envp[++idx])
//	//	printf("main envp : %p\n", bundle.envp[idx]);
//	//printf("%s\n", bundle.envp[0]); //here NULL error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//	pipe2.pipe = NULL;
//	pipe2.cmd = &cmd2;
//	cmd2.redirect_s = NULL;
//	cmd2.simple_cmd = &smp_cmd2;
//	smp_cmd2.cmd_path = "sleep";
//	char *argv_custom2[3] = {"sleep", "3", NULL};
//	smp_cmd2.cmd_argv = argv_custom2;

//	pre_exec_here_doc(&root, &bundle);
//	exec_recur(&root, &bundle, -1, 0);
//	//free_remainders(&bundle);

	free_bundle(&bundle);
	return (0);
}

//1. if child process has an error, main should export $? equals to child process exit status
//	and return (-1) until next readline() start
//2. if redirection has an error, err msg prints and don't exec remain redirections
//3. whenever child process ends, export $?

//어 떨 때 바 로 return return해서 readline 해야 되고 어 떨 때 다음 command가 실행되어야하는지 파악하기




//builtin
//1. echo with -n
//2. cd
//3. pwd
//4. export
//5. unset
//6. env
//7. exit