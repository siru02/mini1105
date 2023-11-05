/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   search_tree.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hyuim <hyuim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/01 21:19:25 by hyuim             #+#    #+#             */
/*   Updated: 2023/11/02 16:57:42 by hyuim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

int	print_2d_str(char **str_arr)
{
	int i = -1;
	while (str_arr[++i])
	{
		if (write(STDOUT_FILENO, str_arr[i], ft_strlen(str_arr[i])) == -1)
			return (1);
		if (write(STDOUT_FILENO, "\n", 1) == -1)
			return (1);
	}
	return (0);
}

int	exec_recur(t_pipe *root, t_bundle *bundle, int before_fd_read, int cmd_idx)
{
	//sleep(1);//////////////////////////
	//write(2, "\n", 1);
	//write(2, "exec recur\n", 11);/////////////////////////////////////
	int	pid;
	int	idx;

	if (pipe(bundle->fd) == -1)
		return (-1);
	if (cmd_idx > 0 && bundle->current_cnt)
	{
		idx = -1;
		while (++idx < bundle->current_cnt->cnt)
			bundle->current_filename = bundle->current_filename->next;
		bundle->current_cnt = bundle->current_cnt->next;
	}
	else
	{
		bundle->current_filename = bundle->hrdc_filename_list;
		bundle->current_cnt = bundle->hrdc_cnt_list;
	}
	pid = fork();
	if (pid == -1) //-1 return -> bundle->err_msg set
		return (-1);
	//------------child-----------------
	if (pid == 0 && root->cmd)
	{
		//write(2, "********child process\n", 22);
		//int before_stdin = dup(STDIN_FILENO);
		if (exec_cmd(root->cmd, bundle, before_fd_read, cmd_idx) == -1)
		{
			//write(2, "plz help\n", 9);
			//dup2(before_stdin, STDIN_FILENO);
			//close(before_stdin);
			return (-1);
		}
		//write(2, "plz help2\n", 10);
		//dup2(before_stdin, STDIN_FILENO);
		//close(before_stdin);
		return (0);
	}
	//-----------------------------------

	//=============parent================
	if (root->pipe == NULL)
		bundle->last_pid = pid;
	close(bundle->fd[1]);
	if (pid > 0 && root->pipe)
		if (exec_recur(root->pipe, bundle, bundle->fd[0], cmd_idx + 1) == -1)
			return (-1);
	close(before_fd_read);
	return (0);
	//===================================
}

void	base_redir_first_cmd(t_bundle *bundle)
{
	//write(2, "base redir first cmd\n", 21);//////////////////////////
	//printf("cmd cnt : %d\n", bundle->cmd_cnt);
	if (bundle->cmd_cnt > 1)
		if (dup2(bundle->fd[1], STDOUT_FILENO) == -1)
			ft_error(DUP2_ERRMSG, 1234);
	if (close(bundle->fd[0]) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
	if (close(bundle->fd[1]) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
}

void	base_redir_mid_cmd(t_bundle *bundle, int before_fd_read)
{
	//write(2, "base redir mid cmd\n", 19);//////////////////////////
	if (dup2(before_fd_read, STDIN_FILENO) == -1)
		ft_error(DUP2_ERRMSG, 1234);
	if (dup2(bundle->fd[1], STDOUT_FILENO) == -1)
		ft_error(DUP2_ERRMSG, 1234);
	if (close(before_fd_read) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
	if (close(bundle->fd[0]) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
	if (close(bundle->fd[1]) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
}

void	base_redir_last_cmd(t_bundle *bundle, int before_fd_read)
{
	//write(2, "base redir last cmd\n", 20);//////////////////////////
	if (dup2(before_fd_read, STDIN_FILENO) == -1)
		ft_error(DUP2_ERRMSG, 1234);
	if (close(before_fd_read) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
	if (close(bundle->fd[0]) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
	if (close(bundle->fd[1]) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
}

int	exec_cmd(t_cmd *cmd, t_bundle *bundle, int before_fd_read, int cmd_idx)
{
	//write(2, "exec cmd\n", 9);//////////////////////////////////
	// base redirection
	if (cmd_idx == 0)
		base_redir_first_cmd(bundle);
	else if (cmd_idx == bundle->cmd_cnt - 1)
		base_redir_last_cmd(bundle, before_fd_read);
	else
		base_redir_mid_cmd(bundle, before_fd_read);

	// exec input redirect
	if (cmd->redirect_s)
		exec_redirect_s_recur(cmd->redirect_s, bundle);
	if (bundle->cmd_cnt == 0)
		exit(0); ///1102
	// exec cmd
	if (!bundle->err_flag && cmd->simple_cmd->cmd_path)
	{
		//printf("wtf\n");
		exec_simple_cmd(cmd->simple_cmd, bundle);
	}
	return (0);
}

void	exec_redirect_s_recur(t_redirect_s *redirect_s, t_bundle *bundle)
{
	//write(2, "exec redirect_s recur\n", 22);////////////////////////////
	if (redirect_s && redirect_s->redirect)
		exec_redirect(redirect_s->redirect, bundle);
	if (redirect_s && redirect_s->redirect_s)
		exec_redirect_s_recur(redirect_s->redirect_s, bundle);
}

void	select_err_msg(char *cmd_path)
{
	if (ft_strlen(cmd_path) >= 2 && cmd_path[0] == '.' && cmd_path[1] == '/')
	{
		ft_error(cmd_path + 2, 126);
	}
	write(2, cmd_path, ft_strlen(cmd_path));
	write(2, ": command not found\n", 20);
	exit(127);
}

void	exec_simple_cmd(t_simple_cmd *simple_cmd, t_bundle *bundle)
{
	//write(2, "exec_simple_cmd\n", 16);//////////////////////////////////
	char	**parsed_path;
	char	*temp;
	char	*new_path;
	int		idx;
	

	if (is_builtin(simple_cmd->cmd_path))
	{
		//write(2, "exec builtin\n", 13);
		exit(exec_builtin(simple_cmd->cmd_path, simple_cmd->cmd_argv, bundle));
	}
	//should split path here bc of export and unset
	//write(2, "exec execve\n", 12);
	if (!access(simple_cmd->cmd_path, F_OK))
	{
		//write(2, "exec simple cmd errrr0\n", 23);////////////////////////////
		if (execve(simple_cmd->cmd_path, simple_cmd->cmd_argv, bundle->envp) == -1)
		{
			//write(2, "exec simple cmd errrr\n", 22);////////////////////////////
			select_err_msg(simple_cmd->cmd_path);
		}
	}
	idx = -1;
	parsed_path = get_path(bundle->envp);
	if (!parsed_path)
	{
		write(2, simple_cmd->cmd_path, ft_strlen(simple_cmd->cmd_path));
		write(2, ": command not found\n", 20);
		exit(127);
	}
	//write(2, "exec simple cmd errrr2\n", 23);////////////////////////////
	while (parsed_path[++idx])
	{
		temp = ft_strjoin(parsed_path[idx], "/");
		if (!temp)
			ft_error(MALLOC_ERRMSG, 1);
		new_path = ft_strjoin(temp, simple_cmd->cmd_path);
		free(temp);
		if (!new_path)
			ft_error(MALLOC_ERRMSG, 1);
		if (!access(new_path, F_OK))
			if (execve(new_path, simple_cmd->cmd_argv, bundle->envp) == -1)
				select_err_msg(simple_cmd->cmd_path);
		free(new_path);
	}
	free_2d_malloced_str(parsed_path);
	write(2, "exec simple cmd error\n", 22);////////////////////////////
	select_err_msg(simple_cmd->cmd_path);
}

char	**get_path(char **envp)
{
	int	idx;
	char	**temp_split;
	char	**splited_path;

	idx = -1;
	//printf("here in gp 1\n"); //////////////////////////////////
	//printf("envp[0] : %p\n", envp);//////////////////////////////////;
	//write(1, envp[0], ft_strlen(envp[0]));
	//printf("hi\n");
	while (envp[++idx])
	{
		if (ft_strlen(envp[idx]) >= 5 && ft_strncmp(envp[idx], "PATH=", 5) == 0)
			break ;
	}
	if (!envp[idx])
		return (NULL);
	temp_split = ft_split(envp[idx], '=');
	if (!temp_split)
		ft_error(MALLOC_ERRMSG, 1234);
	splited_path = ft_split(temp_split[1], ':');
	free_2d_malloced_str(temp_split);
	if (!splited_path)
		ft_error(MALLOC_ERRMSG, 1234);
	return (splited_path);
}

void	free_2d_malloced_str(char **str_arr)
{
	int	idx;

	idx = -1;
	while (str_arr[++idx])
		free(str_arr[idx]);
	free(str_arr);
}

void	exec_redirect(t_redirect *redirect, t_bundle *bundle)
{
	//write(2, "exec redierct\n", 14);//////////////////////////////////////////
	if (redirect->type == REDIR_LEFT)
		redir_left(redirect->filename);
	else if (redirect->type == REDIR_RIGHT)
		redir_right(redirect->filename);
	else if (redirect->type == REDIR_TWO_LEFT)
		redir_two_left(bundle);
	else if (redirect->type == REDIR_TWO_RIGHT)
		redir_two_right(redirect->filename);
}

void	redir_left(char *filename)
{
	//write(2, "redir_left\n", 11);////////////////////////////////////
	int	infile_fd;

	if (access(filename, F_OK))
	{
		write(2, filename, ft_strlen(filename));
		write(2, ": No such file or directory\n", 28);
		exit(1);
	}
	infile_fd = open(filename, O_RDONLY);
	if (infile_fd == -1)
		ft_error(filename, 1234);
	if (dup2(infile_fd, STDIN_FILENO) == -1)
		ft_error(DUP2_ERRMSG, 1234);
	if (close(infile_fd) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
}

void	redir_right(char *filename)
{
	//write(2, "redir_right\n", 12);///////////////////////////////
	int	outfile_fd;
	int	open_flag;
	//char	*err_msg; // if error occured, exit? or err_msg set and then return return return?

	//if (access(filename, F_OK))
	//{
	//	write(2, filename, ft_strlen(filename));
	//	write(2, ": No such file or directory\n", 28);
	//	exit(1);
	//}
	open_flag = O_RDWR | O_CREAT | O_TRUNC;
	outfile_fd = open(filename, open_flag, 0644);
	if (outfile_fd == -1)
		ft_error(filename, 1);
	if (dup2(outfile_fd, STDOUT_FILENO) == -1)
		ft_error(DUP2_ERRMSG, 1);
	if (close(outfile_fd) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
}

void	redir_two_left(t_bundle *bundle)
{
	//write(2, "redir_two_left\n", 15);////////////////////////////////
	int	infile_fd;
	int	open_flag;

	open_flag = O_RDWR | O_CREAT;
	infile_fd = open(bundle->current_filename->content, open_flag, 0644);
	if (infile_fd == -1)
		ft_error(OPEN_ERRMSG, 1234);
	if (dup2(infile_fd, STDIN_FILENO) == -1)
		ft_error(DUP2_ERRMSG, 1234);
	if (close(infile_fd) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
	bundle->current_filename = bundle->current_filename->next;
}

void	redir_two_right(char *filename)
{
	//write(2, "redir_two_right\n", 16);///////////////////////////////////
	int	outfile_fd;
	int	open_flag;

	if (access(filename, F_OK))
	{
		write(2, filename, ft_strlen(filename));
		write(2, ": No such file or directory\n", 28);
		exit(1);
	}
	open_flag = O_RDWR | O_CREAT | O_APPEND;
	outfile_fd = open(filename, open_flag, 0644);
	if (outfile_fd == -1)
		ft_error(filename, 1234);
	if (dup2(outfile_fd, STDOUT_FILENO) == -1)
		ft_error(DUP2_ERRMSG, 1234);
	if (close(outfile_fd) == -1)
		ft_error(CLOSE_ERRMSG, 1234);
}
