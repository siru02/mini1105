/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokenize_no_space.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgu <hgu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/01 16:18:40 by hgu               #+#    #+#             */
/*   Updated: 2023/11/05 19:18:40 by hgu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/minishell.h"

void	check_leak(void)
{
	system("leaks a.out");
}

void	print_simple_cmd(t_simple_cmd *node)
{
	printf("simple_cmd\n\n");
	printf("cmd : %s\n",node->cmd_path);
	for (int i = 0 ; i< node->argv_cnt; i++)
		printf("argv %d : %s\n",i,node->cmd_argv[i]);
	printf("\n\n");
}

void 	print_redirect_s(t_redirect_s *node)
{
	printf("redirect_s\n\n");
	if (node->redirect)
		printf("type : %d, file : %s\n", node->redirect->type, node->redirect->filename);
	if (node->redirect_s)
		print_redirect_s(node->redirect_s);
	printf("\n\n");
}

void	print_cmd(t_cmd *node)
{
	printf("cmd\n\n");
	if (node->redirect_s)
		print_redirect_s(node->redirect_s);
	if (node->simple_cmd)
		print_simple_cmd(node->simple_cmd);
	printf("\n\n");
}

void	print_tree(t_pipe *node, t_bundle *bundle)
{
	printf("pipe -> cmd = %d\n\n",bundle->cmd_cnt);
	if (node->cmd != NULL)
		print_cmd(node->cmd);
	if (node->pipe != NULL)
		print_tree(node->pipe, bundle);
	printf("\n\n");
}


t_token	*free_all_token(t_token *token_head)
{
	t_token	*tmp;
	t_token	*to_free;

	tmp = token_head;
	while (tmp != NULL)
	{
		to_free = tmp;
		tmp = tmp->next;
		free(to_free->value);
		free(to_free);
	}
	return (NULL);
}

void	make_token(char *str, int len, int type, t_token **head)
{
	t_token	*new; //새로 만들 토큰
	t_token	*tmp; //헤드순회에 쓰일임시 변수
	int		idx; 

	new = malloc(sizeof(t_token)); //새로만들 토큰을 말록해준다
	new->next = NULL; //new의 next는 NULL이다
	new->type = type; //새로만든 노드의 타입지정
	new->expansion_cnt = 0;//1105
	new->quote_end = -1;//1105
	new->quote_start = -1;//1105
	if (*head == NULL) //head가 비어있으면
	{
		*head = new; //새로만든 노드를 헤드에 할당한다
		(*head)->value = NULL; //헤드의 value는 NULL이고
		return ; //헤드인경우 종료
	}
	tmp = *head;
	while (tmp->next != NULL) //tmp->next == NULL일때까지 tmp순회
		tmp = tmp->next;
	tmp->next = new; //새로만든 노드를 붙인다
	new->value = malloc(sizeof(char) * (len + 2)); //새로운 토큰의 문자열
	idx = -1;
	ft_strlcpy(new->value, str, len + 2); //len은 복사할 문자의 idx이므로 \0까지 생각해서 len + 2
	//printf("type : %d len :%d new : %s\n",type, len, new->value);
}

int	check_ch_is_token(char *str)
{
	if (*str == ' ' || *str == 9 || *str == 10 || *str == 11 || *str == 12 || *str == 13) //현재문자가 공백이면
		return (WHITE_SPACE); //white_space 
	else if (*str == '>') //리다이렉션 > 기호이면 다음 기호까지 확인한다
	{
		if (*(str + 1) == '>') //>>이면
			return (REDIR_TWO_RIGHT);
		return (REDIR_RIGHT); //>>이 아니면
	}
	else if (*str == '<')
	{
		if (*(str + 1) == '<')
			return (REDIR_TWO_LEFT);
		return (REDIR_LEFT);
	}
	else if (*str == '\'') //작은 따옴표
		return (S_QUOTATION);
	else if (*str == '\"') //큰 따옴표
		return (D_QUOTATION);
	else if (*str =='|') //PIPE
		return (PIPE);
	else if (*str == '\0') //문자마지막이면
		return (100);
	return (-1); //word
}

//앞뒤문자가 공백이 아니면 앞뒷문자도 붙여준다 //unclosed quote를 처리해주기
//1007. 따옴표로 끝난경우 마지막 부분이 tokenize되지않는다 -> tokenize단계에서 free해주고, error출력하고, root는 NULL로 반환 
int	jump_quote(char *s, int idx)
{ //따옴표가 끝나지 않으면 NULL을 보낸다?, 닫히지 않은 quote를 처리하지 않아도 된다
	if (s[idx] == '\'') //순회중에 작은 따옴표를 찾으면
	{
		while (s[idx + 1] && s[idx + 1] != '\'') //무지성 인덱스 증가 //str[idx + 1]는 '나 \0을 가리킨상태로멈추게된다
			idx++;
		if (s[idx + 1] == '\0') //
			write(2, "error1\n", 7);//fd 2로 쏘기
	}
	else if (s[idx] == '\"') //순회중에 큰 따옴표를 찾으면 
	{
		while (s[idx + 1] && s[idx + 1] != '\"') //무지성 인덱스 증가
			idx++;
		if (s[idx + 1] == '\0') //
			write(2, "error2\n", 7);//fd 2로 쏘기
	}
	if (s[idx + 1] == '\0')
		return (-1);
	return (idx + 1); // quote나 \0을 가리키는 인덱스를 반환한다
}

 //현재 오류가 있음 -> "|의 형태에서 "가 삭제된다
t_token	*tokenize(char *str)
{
	int		type; //각  token의 type값으로 -1이면 덩어리 멤버
	int		len; //각 token의 문자 길이
	t_token	*token_head;

	len = -1;
	token_head = NULL;
	make_token(NULL, 0, PIPE, &token_head);
	while (*(str + ++len))//len은 실제 구분자 길이보다 1 짧다
	{
		type = check_ch_is_token(str + len);
		if (type == S_QUOTATION || type == D_QUOTATION) //현재문자의 타입이 작은따옴표나 큰 따옴표이면
			len = jump_quote(str, len); //만약 quote가 닫히지않았으면 -1반환
		if (len == -1) //따옴표가 닫히지 않은경우
			return (free_all_token(token_head)); //모든걸 프리해주고 NULL을 반환한다
		if (type == WHITE_SPACE)
		{
			str++;
			len--;
		}
		if ((type <= 1 && check_ch_is_token(str + len + 1) > 1) || type > 3)//현재문자가 단어이고, 다음 문자가 파이프, 세미콜론, 리다이렉션, \0이면
		{
			if (type == REDIR_TWO_LEFT || type == REDIR_TWO_RIGHT)//>>, <<의 경우
				len++; //길이를 하나 더 늘려준다
			make_token(str, len, type, &token_head);
			str = str + len + 1;
			len = -1;
		}
	}
	if (type != WHITE_SPACE && len != 0)
		make_token(str, --len, type, &token_head);
	return (token_head);
}

//int main()
//{
//	t_token	*head;
//	t_token	*tmp;
//	//t_token *to_free;
//	t_pipe	*root;
//	char	*ret;
//	t_bundle *bundle;

//	//atexit(check_leak);
//	bundle = malloc(sizeof(t_bundle));
//	while (1)
//	{
//		bundle->cmd_cnt = 0;
//		ret = readline("hgu hi : ");
//		head = tokenize(ret);
//		tmp = head;
//		if (head == NULL)
//			printf("successfully freed\n");
//		else
//			tmp = syntax_analyze(head);
//		if (tmp == NULL)
//		{
//			printf("syntax error\n");
//			free(ret);
//			continue;
//		}
//		printf("여기까지 동작 : syntax OK\n");
//		root = make_tree(head, bundle);
//		print_tree(root, bundle);
//		// while (tmp != NULL)
//		// {
//		// 	printf("type : %d len : %zu new : %s\n",tmp->type,ft_strlen(tmp->value), tmp->value);
//		// 	// write (1, tmp->value, ft_strlen(tmp->value));
//		// 	// write (1, "\n", 1);
//		// 	to_free = tmp;
//		// 	tmp = tmp->next;
//		// 	free(to_free->value);
//		// 	free(to_free);
//		// }
//		printf("여기오냐?\n");
//		free(ret);
//		// return(0);
//	}
//}

//따옴표가 나오면 끝마치는 따옴표가 나올때까지 한 토큰으로 처리한다'
//앞뒤에 공백이 없으면 붙어있는 문자도 같은 토큰으로 인식한다
