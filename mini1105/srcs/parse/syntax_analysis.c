/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syntax_analysis.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgu <hgu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/07 21:50:39 by hgu               #+#    #+#             */
/*   Updated: 2023/11/03 22:07:20 by hgu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

int	syntax_error(char *str)  //출력할 오류문구를 입력받아 출력하고, -1리턴
{
	write(2, "bash: syntax error near unexpected token `", 42);
	write(2, str, ft_strlen(str));
	write(2, "\'\n", 2);
	return (-1);
}

//각 파이프마다 첫 토큰은 명령어나 리다이렉션만 올 수 있으므로, word인 경우 command로 타입을 변경해줌
int	syntax_first_token(t_token *first, int cnt)
{ //cnt는 0이면 헤드토큰임을 의미한다
	if (cnt == 0 && first->next == NULL) //헤드 토큰 = pipe, 다음 토큰 비어있음
		return (1); //정상이고 1반환 아무것도 실행하지않음
	else if (cnt == 0 && first->next->type == PIPE)
		return (syntax_error("|"));
	// else if (first->type == -1) //word는 cmd, filename, option으로 분류되는데 첫커맨드가 리다이렉션이 아니면 항상 cmd로 인식한다
	// {
	// 	first->type = COMMAND;
	// 	return (1);
	// }
	if (first->next == NULL) //2번째 파이프부터 파이프 -> NULL의 꼴이면
		return (syntax_error("|"));
	else if (first->next->type <= 1 ) //다음이 word이면
		return (1);//정상
	else if (first->next->type >= REDIR_LEFT && first->next->type <= REDIR_TWO_RIGHT) //다음이 리다이렉트이면 정상
		return (1);
	return (syntax_error(first->next->value)); //임시로 첫 인자가 word거나 리다이렉션이 아닌경우 에러로 리턴한다
}
int	syntax_redirection(t_token *token)
{ //현재토큰이 리다이렉션인경우 바로 다음에 word타입이 와야한다
	t_token	*tmp;

	tmp = token;
	if (tmp->type >= REDIR_LEFT && tmp->type <= REDIR_TWO_RIGHT) //현재 토큰타입이 리다이렉션인경우
	{
		tmp = tmp->next;
		if (tmp == NULL) //리다이렉션 - 빈토큰 의 형태이면
			return (syntax_error("newline")); //오류 반환
		else if (tmp->type != -1) //리다이렉션 - word의 형태가 아니면
			return (syntax_error(tmp->value));
		tmp->type = FILENAME; //리다이렉션의 뒤 이므로 filename형식으로 저장
	}
	return (0); //정상적인 경우는 0리턴
}

//앞의 토큰과 대조해서 현재토큰에 타입을 지정해준다
// 파이프바로뒤의 토큰은 '리다이렉션' or '커맨드' 이다
// 리다이렉션 바로뒤의 토큰은 filename만 온다
// cmd뒤의 토큰들은 파이프나 리다이렉션이 오기 전까지 word로 취급하며, argv에 문자열배열로 들어간다
// 명령어뒤의 부적절한 리다이렉션은 오류를 newline으로 뱉어낸다
// 여기서 모든 문법적 오류들을 처리한다
// 오류가 있으면 NULL리턴, 문법오류없으면 head의 주소리턴
t_token	*syntax_analyze(t_token *token_head)
{//첫 파이프가 나오기이전은 처음부터 읽지만 파이프만난 이후는 파이프 이후부터 읽는 문제가 있다
	t_token	*tmp;
	int		cnt;

	tmp = token_head; //토큰의 헤드는 pipe타입이다 //quote가 닫히지 않으면 NULL포인터가 들어온다
	if (tmp == NULL)
		return (NULL);
	cnt = 0;
	while (tmp != NULL)
	{
		if (syntax_first_token(tmp, cnt++) == -1)//매 파이프마다 첫 토큰은 리다이렉션과 명령어만 올 수 있다
			return (free_all_token(token_head));
		tmp = tmp->next; //파이프로 잘린노드의 다음 노드부터 검사하므로
		while(tmp != NULL) //현재 토큰의 타입이 파이프가 아닌동안 의미를 부여한다 //pipe나 NULL에서 탈출
		{
			if (tmp->type == PIPE)
				break;
			if (syntax_redirection(tmp) == -1) //tmp의 type이 redirection인 경우 문법검사를 수행한다
				return (free_all_token(token_head));
			//printf("tokenized str : %s\n",tmp->value);
			tmp = tmp->next; //tmp == NULL 이거나 tmp->type == PIPE에서 탈출
		}
	}
	return (token_head); //문제없는 경우는 원래 토큰의 head를 리턴한다
}
