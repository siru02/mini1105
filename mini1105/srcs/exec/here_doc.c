/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   here_doc.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hyuim <hyuim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/11 15:52:35 by hyuim             #+#    #+#             */
/*   Updated: 2023/11/07 11:40:19 by hyuim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

int	pre_exec_here_doc(t_pipe *root, t_bundle *bundle)
{
	t_list_hrdc	*new;

	if (root->cmd->redirect_s)
	{
		new = ft_lstnew_hrdc();
		if (!new)
			ft_error(MALLOC_ERRMSG, 1234);
		ft_lstadd_back_hrdc(&(bundle->hrdc_cnt_list), new);
		if (search_here_doc(root->cmd->redirect_s, bundle, new) == -1)
			return (-1);
	}
	if (root->pipe)
		if (pre_exec_here_doc(root->pipe, bundle) == -1)
			return (-1);
	return (0);
}

t_list_hrdc	*ft_lstnew_hrdc(void)
{
	t_list_hrdc	*new_list;

	new_list = (t_list_hrdc *)malloc(sizeof(t_list_hrdc));
	if (!new_list)
		return (NULL);
	new_list->cnt = 0;
	new_list->next = NULL;
	return (new_list);
}

void	ft_lstadd_back_hrdc(t_list_hrdc **lst, t_list_hrdc *new)
{
	t_list_hrdc	*temp;

	if (!*lst)
	{
		*lst = new;
		return ;
	}
	temp = *lst;
	while (temp->next)
	{
		temp = temp->next;
	}
	temp->next = new;
	return ;
}

int	search_here_doc(t_redirect_s *redirect_s, t_bundle *bundle, t_list_hrdc *new)
{
	if (redirect_s->redirect->type == REDIR_TWO_LEFT)
	{
		new->cnt++;
		if (exec_here_doc(redirect_s->redirect->filename, bundle) == -1)
			return (-1);
	}
	if (redirect_s->redirect_s)
		if (search_here_doc(redirect_s->redirect_s, bundle, new) == -1)
			return (-1);
	return (0);
}

int	exec_here_doc(char *eof, t_bundle *bundle) //here_doc
{
	//write(2, "exec here doc\n", 14);////////////////////////////////////
	char	*temp_filename;
	char	*temp;
	int		tempfile_fd;
	int		open_flag;
	t_list	*new_node;

	temp_filename = ft_strdup("/tmp/tmp");
	if (!temp_filename)
		ft_error(OPEN_ERRMSG, 1234);
	open_flag = O_RDWR | O_CREAT;
	while (!access(temp_filename, F_OK))
	{
		temp = ft_strjoin(temp_filename, "tmp");
		if (!temp)
			ft_error(MALLOC_ERRMSG, 1234);
		free(temp_filename);
		temp_filename = temp;
	}
	tempfile_fd = open(temp_filename, open_flag, 0644);
	if (write_to_tempfile_until_eof(eof, tempfile_fd) == -1)
	{
		close(tempfile_fd);
		return (-1);
	}
	if (tempfile_fd == -1)
	{
		ft_error(OPEN_ERRMSG, 1234);
	}
	close(tempfile_fd);
	new_node = ft_lstnew(temp_filename);
	if (!new_node)
		ft_error(MALLOC_ERRMSG, 1234);
	ft_lstadd_back(&(bundle->hrdc_filename_list), new_node);
	return (0);
}

void	sigint_handler_during_heredoc()
{
	exit_status = -1;
	close(STDIN_FILENO);
}

void	sigquit_handler_during_heredoc()
{
	return ;
}

int	write_to_tempfile_until_eof(char *eof, int tempfile_fd)
{
	//write(2, "write to tempfile until eof\n", 28);///////////////////////////
	char	*inp;
	size_t	inp_len;
	size_t	eof_len;
	char	*add_newline;
	int		before_stdin;

	eof_len = ft_strlen(eof);
	before_stdin = dup(STDIN_FILENO);
	signal(SIGINT, sigint_handler_during_heredoc);
	signal(SIGQUIT, sigquit_handler_during_heredoc);
	while (exit_status >= 0)
	{
		inp = readline("> ");
		if (exit_status == -1)
			break ;
		inp_len = ft_strlen(inp);
		if (inp == NULL || ((inp_len == eof_len) && !ft_strcmp(inp, eof)))
		{
			free(inp);
			break ;
		}
		add_newline = ft_strjoin(inp, "\n");
		free(inp);
		inp = NULL;
		if (!add_newline)
			ft_error(MALLOC_ERRMSG, 1234);
		if (write(tempfile_fd, add_newline, inp_len + 1) == -1)
			ft_error(WRITE_ERRMSG, 1234);
		free(add_newline);
		add_newline = NULL;
	}
	signal(SIGINT, sigint_handler);
	signal(SIGQUIT, sigquit_handler);
	dup2(before_stdin, STDIN_FILENO);
	close(before_stdin);
	if (exit_status < 0)
	{
		exit_status *= -1;
		return (-1);
	}
	//if (exit_status < 0)
	//	return (-1);
	return (0);
}
