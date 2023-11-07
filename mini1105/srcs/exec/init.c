/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hyuim <hyuim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/05 16:14:06 by hyuim             #+#    #+#             */
/*   Updated: 2023/11/06 19:14:44 by hyuim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

void	sigint_handler()
{
	exit_status = 1;
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

void	sigint_handler_during_fork()
{
	write(1, "\n", 1);
	exit_status = 1;
	return ;
}

void	sigquit_handler()
{
	return ;
}

void	sigquit_handler_during_fork()
{
	write(1, "Quit: 3\n", 7);
	exit_status = 131;
}

void	init_bundle(t_bundle *bundle)
{
	bundle->hrdc_filename_list = NULL;
	bundle->hrdc_cnt_list = NULL;
	bundle->fd[0] = -42;
	bundle->fd[1] = -42;
	bundle->err_msg = NULL;
	bundle->err_flag = 0;
	bundle->cmd_cnt = 0;
	bundle->last_pid = -1;
	signal(SIGINT, sigint_handler);
	signal(SIGQUIT, sigquit_handler);
}

void	cp_envp(t_bundle *bundle, char *envp[])
{
	int		envp_len;
	int		idx;
	char	**malloced_envp;

	envp_len = -1;
	while (envp[++envp_len])
		;
	malloced_envp = (char **)malloc(sizeof(char *) * (envp_len + 1));
	if (!malloced_envp)
		ft_error("malloc error ", 1234);
	malloced_envp[envp_len] = NULL;
	idx = -1;
	while (++idx < envp_len)
	{
		malloced_envp[idx] = ft_strdup(envp[idx]);
		if (!malloced_envp)
			ft_error("malloc error ", 1234);
	}
	bundle->envp = malloced_envp;
	bundle->envp_len = envp_len;
}