/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   make_tree.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgu <hgu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/11 19:25:06 by hgu               #+#    #+#             */
/*   Updated: 2023/11/05 19:44:52 by hgu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minishell.h"

void	copy_strings(char **new, char **old, int cnt)
{
	int	idx;

	idx = 0;
	while (idx < cnt)
	{
		new[idx] = old[idx];
		idx++;
	}
}

void	make_simple_cmd(t_cmd *cmd, t_token *token, t_bundle *bundle)
{ //cmd_argv는 char**형식이다 //simple_cmd의 argv에 값을 이어붙인다
	t_simple_cmd	*tmp;
	char			**new_argv;

	tmp = cmd->simple_cmd;
	if (tmp->cmd_path == NULL) //명령어 저장부분이 비어있다면
	{
		tmp->cmd_path = token->value; //명령어를 추가해준다
		bundle->cmd_cnt++;
	}
	tmp->argv_cnt += 1;
	new_argv = malloc(sizeof(char *) * (tmp->argv_cnt + 1)); //새로운 2차원배열 만들어준다
	copy_strings(new_argv, tmp->cmd_argv, tmp->argv_cnt - 1); //이전까지내용 복사하고
	free(tmp->cmd_argv); //기존에 있던거 프리
	tmp->cmd_argv = new_argv;//새로운 내용을 뒤에 붙여주고
	tmp->cmd_argv[tmp->argv_cnt - 1] = token->value;
	tmp->cmd_argv[tmp->argv_cnt] = NULL; //마지막인덱스에 NULL포인터를 넣어준다
}

t_cmd	*make_cmd(void)
{ //cmd를 만들어서 return 한다
	t_cmd			*cmd;
	t_simple_cmd	*simple;

	cmd = malloc(sizeof(t_cmd));
	simple = malloc(sizeof(t_simple_cmd));
	simple->cmd_argv = NULL;
	simple->cmd_path = NULL;
	cmd->redirect_s = NULL;
	cmd->simple_cmd = simple;
	cmd->simple_cmd->argv_cnt = 0;
	return (cmd);
}

t_pipe	*make_pipe(void)
{ //token을 받아서 파이프 생성한다
	t_pipe	*pipe;

	pipe = malloc(sizeof(t_pipe)); //파이프노드생성
	pipe->cmd = make_cmd(); //make_cmd는 cmd노드를 만들어서 리턴한다
	pipe->pipe = NULL; //다음파이프는 NULL로 채워준다
	return (pipe);
}
t_redirect	*make_redirect(t_token *token)
{//현재토큰(리다이렉트)을 인자로 받는다
	t_redirect	*new;

	new = malloc(sizeof(t_redirect));
	new->type = token->type;
	new->filename = token->next->value; //리다이렉트노드 다음토큰에 파일이름저장되어있으므로
	free(token->value);
	return (new);
}

void	make_redirect_s(t_redirect_s **head, t_token *token)
{ //현재 파이프의 리다이렉츠 헤드를 넘겨준다
	t_redirect_s	*new; //새로만들어줄 노드
	t_redirect_s	*tmp; //redirect_s를 탐색할 임시변수

	new = malloc(sizeof(t_redirect_s));
	new->redirect = make_redirect(token); //새노드의 리다이렉트노드에 값을 추가한다
	new->redirect_s = NULL; //새로만든 노드의 next는 NULL
	if (*head == NULL) //헤드가 비어있으면
		*head = new; //새로만든 노드를 헤드로
	else //헤드가 비어있지않으면 새로만든걸 뒤에 붙여준다
	{
		tmp = *head;
		while (tmp->redirect_s)
			tmp = tmp->redirect_s; //노드의 next가 NULL일때까지 계속 탐색한다
		tmp->redirect_s = new;
	}
}

t_pipe	*make_tree(t_token *token_head, t_bundle *bundle)
{
	t_token	*tmp; //토큰순회
	t_pipe	*pipe_head; //파이프루트저장
	t_pipe	*pipe; //파이프순회

	tmp = token_head->next; //head의 속성은 pipe이므로
	pipe_head = make_pipe(); //루트파이프를 생성한다
	pipe = pipe_head;
	while (tmp) //token을 끝까지 탐색한다
	{
		if (tmp->type == PIPE) //헤드이거나 PIPE이면
		{
			free(tmp->value);
			pipe->pipe = make_pipe(); //새로운 파이프 생성
			pipe = pipe->pipe;
		}
		else if (tmp->type >= REDIR_LEFT && tmp->type <= REDIR_TWO_RIGHT) //리다이렉션이면
		{
			make_redirect_s(&(pipe->cmd->redirect_s), tmp); //syntax단계에서 리다이렉션뒤에는 파일이름이 있는지 검사하므로
			tmp = tmp->next; //다음토큰으로 넘긴다
		}
		else //나머지 word의 경우
			make_simple_cmd(pipe->cmd, tmp, bundle);//커맨드를 한번 받았으면 이후로는 argv에넣어야한다
		tmp = tmp->next; //다음 토큰으로
	}
	return (pipe_head);
}
