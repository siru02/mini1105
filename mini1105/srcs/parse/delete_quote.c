/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   delete_quote.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgu <hgu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/02 19:27:47 by hgu               #+#    #+#             */
/*   Updated: 2023/11/05 22:26:32 by hgu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

void	cut_quote(int open, int close, t_token *token)
{
	char	*str;
	int		malloc_size;

	malloc_size = ft_strlen(token->value) - 1; //따옴표 2개를 제거하므로 원래길이에서 1개 감소한다('\0'있으므로)
	str = malloc(ft_strlen(token->value) - 1);
	ft_strlcpy(str, token->value, open + 1); //quote전까지 복사한다
	printf("cut_quote test1 : %s close : %d open : %d\n",str,close, open);
	ft_strlcat(str, token->value + open + 1, ft_strlen(str) + close - open); //문제발생
	printf("cut_quote test2 : %s\n",str); //
	ft_strlcat(str, token->value + close + 1, malloc_size);
	printf("cut_quote test3 : %s\n",str);
	free(token->value);
	token->value = str;
}
// char	*new_cut_quote(int open, int close, char *target)
// {
// 	char	*str;
// 	int		malloc_size;

// 	malloc_size = ft_strlen(target) - 1; //따옴표 2개를 제거하므로 원래길이에서 1개 감소한다('\0'있으므로)
// 	str = malloc(ft_strlen(target) - 1);
// 	ft_strlcpy(str, target, open + 1); //quote전까지 복사한다
// 	printf("cut_quote test1 : %s close : %d open : %d\n",str,close, open);
// 	ft_strlcat(str, target + open + 1, ft_strlen(str) + close - open); //문제발생
// 	printf("cut_quote test2 : %s\n",str); //
// 	ft_strlcat(str, target + close + 1, malloc_size);
// 	printf("cut_quote test3 : %s\n",str);
// 	free(target);
// 	return (str);
// }

// void	del_quote(char *target)
// {
// 	int		idx;
// 	char	ch; //만난 따옴표의 종류를 저장한다 작은 따옴표면 작은따옴표로, -> 닫히는 따옴표를 만나면 널문자로 초기화
// 	int		open_quote; //시작하는 quote의 인덱스
// 	int		close_quote; //닫는 quote의 인덱스

// 	idx = -1;
// 	ch = '\0';
// 	while (target[++idx] != '\0')
// 	{
// 		if (ch == '\0' && (target[idx] == '\'' || target[idx] == '\"'))
// 		{
// 			ch = target[idx];
// 			open_quote = idx;
// 		}
// 		else if (ch != '\0' && target[idx] == ch)
// 		{
// 			ch = '\0';
// 			close_quote = idx;
// 			target = new_cut_quote(open_quote, close_quote, target);
// 			idx = close_quote - 2; //단어가 2개 줄어들기 때문에 "test"'hi"같은 경우 test파트의 open = 0 close = 5가 된다
// 		}
// 	}
// }

void	delete_quote_after_expansion(t_token *tk)
{
	int		idx;
	char	ch; //만난 따옴표의 종류를 저장한다 작은 따옴표면 작은따옴표로, -> 닫히는 따옴표를 만나면 널문자로 초기화
	int		open_quote; //시작하는 quote의 인덱스
	int		close_quote; //닫는 quote의 인덱스

	ch = '\0';
	printf("str : %s | expansion_idx : %d\n", tk->value, tk->expansion_idx);
	idx = tk->expansion_idx - 1;
	while (tk->value[++idx] != '\0')
	{
		if (ch == '\0' && (tk->value[idx] == '\'' || tk->value[idx] == '\"'))
		{
			printf("test1\n");
			ch = tk->value[idx];
			open_quote = idx;
			printf("open_quote : %d\n",open_quote);
		}
		else if (ch != '\0' && tk->value[idx] == ch)
		{
			printf("test2 idx : %d\n",idx);
			ch = '\0';
			close_quote = idx;
			cut_quote(open_quote, close_quote, tk);
			printf("test3 : %s %d\n", tk->value, idx);
			idx = close_quote - 2; //단어가 2개 줄어들기 때문에 "test"'hi"같은 경우 test파트의 open = 0 close = 5가 된다
			printf("close_quote : %d\n",close_quote);
		}
	}
	printf("test4\n");//여기로못온다
}


void	delete_quote(t_token *tk)
{//확장이 끝난 토큰을 받아서 따옴표를 제거해준다 //큰따옴표나 작은 따옴표를 만나면 닫히는 따옴표를 만날때까지의 2개의 인덱스를 받는다
	int		idx;
	char	ch; //만난 따옴표의 종류를 저장한다 작은 따옴표면 작은따옴표로, -> 닫히는 따옴표를 만나면 널문자로 초기화
	int		open_quote; //시작하는 quote의 인덱스
	int		close_quote; //닫는 quote의 인덱스

	idx = -1;
	// if (tk->expansion_idx != -1)
	// 	idx = tk->expansion_idx - 1;
	ch = '\0';
	while (tk->value[++idx] != '\0')
	{
		if (ch == '\0' && (tk->value[idx] == '\'' || tk->value[idx] == '\"'))
		{
			printf("test1\n");
			ch = tk->value[idx];
			open_quote = idx;
			printf("open_quote : %d\n",open_quote);
		}
		else if (ch != '\0' && tk->value[idx] == ch)
		{
			printf("test2 idx : %d\n",idx);
			ch = '\0';
			close_quote = idx;
			cut_quote(open_quote, close_quote, tk);
			printf("test3 : %s %d\n", tk->value, idx);
			idx = close_quote - 2; //단어가 2개 줄어들기 때문에 "test"'hi"같은 경우 test파트의 open = 0 close = 5가 된다
			printf("close_quote : %d\n",close_quote);
		}
	}
	printf("test4\n");//여기로못온다
}
