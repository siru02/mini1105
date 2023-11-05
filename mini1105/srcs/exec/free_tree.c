/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_tree.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgu <hgu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/01 17:49:57 by hyuim             #+#    #+#             */
/*   Updated: 2023/11/05 20:14:46 by hgu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

void	free_tree(t_pipe *root)
{
	if (root && root->pipe)
		free_tree(root->pipe);
	if (root && root->cmd)
		free_cmd(root->cmd);
	free(root);
}

void	free_cmd(t_cmd *cmd)
{
	int	idx;

	free(cmd->simple_cmd->cmd_path); //cmd_path와 cmd_argv[0]은 일치한다
	idx = 0;
	//printf("aa : %s\n", cmd->simple_cmd->cmd_argv[0]);
	//free(cmd->simple_cmd->cmd_argv[0]);
	if (cmd->simple_cmd->cmd_argv) //cmd_argv가 존재하면 //1105
		while (cmd->simple_cmd->cmd_argv[++idx] != NULL) //1105
		{
			printf("aa : %s\n", cmd->simple_cmd->cmd_argv[idx]); //1105
			free(cmd->simple_cmd->cmd_argv[idx]); //1105체크 여기서 오류발생
		}
	//printf("ab : %s\n", cmd->simple_cmd->cmd_argv[idx]);
	free(cmd->simple_cmd->cmd_argv);
	free(cmd->simple_cmd);
	if (cmd->redirect_s)
		free_redirect_s(cmd->redirect_s);
	free(cmd);
}

void	free_redirect_s(t_redirect_s *redirect_s)
{
	if (redirect_s->redirect_s)
		free_redirect_s(redirect_s->redirect_s);
	if (redirect_s->redirect)
		free_redirect(redirect_s->redirect);
	free(redirect_s);
}

void	free_redirect(t_redirect *redirect)
{
	if (redirect->filename)
		free(redirect->filename);
	free(redirect);
}

void	free_token(t_token *token)
{
	t_token *temp;

	temp = token;
	while (temp)
	{
		free(temp);
		token = token->next;
		temp = token;
	}
}