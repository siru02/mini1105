/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_expansion.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hgu <hgu@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/04 13:51:10 by hgu               #+#    #+#             */
/*   Updated: 2023/11/07 13:30:13 by hgu              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

t_token	*delete_empty_token(t_token *head)
{//토큰의 헤드는 빈문자열을 들고있다
	t_token	*before;
	t_token	*now;

	now = head->next;
	before = head;
	while (now != NULL)
	{
		printf("empty test : %s\n", now->value);
		if (now->value[0] == '\0')//토큰이 빈 문자열이면
		{
			if (before->type >= 7 && before->type <= 10) //1106 현재 토큰이 빈 문자열인데 이전 토큰이 리다이렉션이면 프리해주고 실행해야한다
			;
			else
			{
				printf("this is empty\n");
				before->next = now->next; //이전노드의 next를 현재노드의 next로 이어준다
				free(now->value); //문자열 free
				free(now); //토큰 free
				now = before;
			}
		}
		before = now;
		now = now->next;
	}
	if (head->next == NULL)
	{
		printf("case free_all\n");
		return (free_all_token(head));
	}
	return (head);
}

int expansion_separator(char ch)
{ //소문자 대문자 숫자 언더바 제외하고는 확장구분자로 사용된다 //$바로뒤에 숫자가 오면 그 숫자랑 $를 같이 지워버린다
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) //대소문자이면
	 	return (0); //구분자가 아니다
	else if (ch >= '0' && ch <= '9') //숫자이면
		return (0);
	else if (ch == '_')//언더바이면	
		return (0);
	return (1); //구분자이다
}

int	cut_env_idx(char *value, int idx)
{ //문자열과 $로 시작하는 시작주소를 받아서 확장해야할 단어의 마지막문자열 주소인덱스를 반환한다 //$abc 이면 $의 주소idx를 받고, c의 주소 idx를 반환하도록 한다
	int	end_point;//echo $USER의 경우 cut이 5가된다

	end_point = idx;
	while (value[++end_point]) // \0에서 반복문 종료
	{
		if (expansion_separator(value[end_point])) //1을 리턴하면 구분자인경우다 //구분자란 (_, 숫자, 영문자)                                                                                                                                       를 제외한 모든 문자
			break ;
	}
	return (end_point - 1);
}

char	*delete_only_dollar(char *value, int dollar)
{
	char	*str;
	int		len;

	len = ft_strlen(value);
	str = malloc(len);
	ft_strlcpy(str, value, dollar + 1);
	ft_strlcat(str, value + dollar + 1, len);
	free(value);
	return (str);
}

int	dollar_with_quote(t_token *token, int close_quote)
{//$"test" //"hi$"test" //전자는 test 후자는 hi$test 
	int		idx;
	char	ch;

	idx = -1;
	ch = '\0';
	printf("close_qtuoe : %d\n",close_quote);
	while ((++idx) < close_quote)//1차로 순회하면서 close_quote앞의 quote쌍들을 제거해준다
	{
		if (ch == '\0' && (token->value[idx] == '\'' || token->value[idx] == '\"'))
		{
			ch = token->value[idx];
			token->quote_start = idx;
		}
		else if (ch != '\0' && token->value[idx] == ch) //닫는 quote를 찾은경우 //수정필요있음
		{
			ch = '\0';
			printf("1 : %d\n",idx);
			token->value = new_cut_quote(token->quote_start, idx, token->value); //quote를 삭제한다
			idx = idx - 2; //단어가 2개 줄어들기 때문에 "test"'hi"같은 경우 test파트의 open = 0 close = 5가 된다
			close_quote -= 2; //quote를 2개 줄였으므로 우리가 찾는 close_quote도 2개 줄여준다
		}
	}
	printf("dollar_with_quote 1차순회 : %s\n", token->value); //echo "test""$"'$fuck'
	idx = -1;
	while (++idx < close_quote)
	{
		if (token->value[idx] == token->value[close_quote]) //일치하는 쌍을 찾으면
		{
			token->value = new_cut_quote(idx, close_quote, token->value); //quote를 제거해주고
			printf("idx < close_quote : %s\n",token->value);
			if (idx < close_quote)//idx가 close_quote보다 작은 경우
				close_quote -= 1;//$의 인덱스가 1개 줄어들기 때문에 계산에 사용할 close_quote도 1개 줄여준다
			return (close_quote - 1);//확장해서 "$"가 $가 된 경우에는 확장하지 않으므로 검사x
		}
	}
	/*
	//"hi$"test 에 대한 처리도 만들어야한다
	*/

	idx = close_quote; //이 경우는 $"test"의 처리
	while (token->value[++idx])//close_quote + 1부터 탐색해서 현재 idx와 쌍을 이루는 quote를 제거한다
	{
		if (token->value[idx] == token->value[close_quote]) //일치하는 쌍을 찾으면
		{
			token->value = new_cut_quote(close_quote, idx, token->value); //quote를 제거해주고
			if (idx < close_quote)//idx가 close_quote보다 작은 경우
				close_quote -= 1;//$의 인덱스가 1개 줄어들기 때문에 계산에 사용할 close_quote도 1개 줄여준다
			token->value = delete_only_dollar(token->value, close_quote - 1); //$를 제거해줘야한다
			break ;
		}
	}
	printf("dollar_with_quote : %s %d\n",token->value, close_quote - 2);
	return (close_quote - 2);
}

int	dollar_with_dollar(t_token *token, int idx)
{//토큰주소와 첫$의 idx를 받음, 앞으로 탐색해야하는 index를 반환해야한다
	char	*pid;
	char	*tmp;
	int		len;
	int		malloc_size;

	pid = ft_itoa(getpid()); //pid를 받아온다
	len = ft_strlen(pid); //pid길이
	malloc_size = ft_strlen(token->value) - 2 + len + 1;
	tmp = malloc(malloc_size); //원래길이 - 2 + 추가할부분 + NULL의 길이
	ft_strlcpy(tmp, token->value, idx + 1);
	ft_strlcat(tmp, pid, ft_strlen(tmp) + len + 1);
	ft_strlcat(tmp, token->value + idx + 2, malloc_size);
	free(token->value);
	free(pid);
	token->value = tmp;
	return (idx + len - 1);//$$test -> 12345test이면 idx = 0 len = 5이고 앞으로 탐색하는 idx - 1을 반환한다
}

int	dollar_with_question_mark(t_token *token, int idx)
{
	char	*exit_code;
		char	*tmp;
	int		len;
	int		malloc_size;

	exit_code = ft_itoa(exit_status);
	len = ft_strlen(exit_code); 
	malloc_size = ft_strlen(token->value) - 2 + len + 1;
	tmp = malloc(malloc_size); //원래길이 - 2 + 추가할부분 + NULL의 길이
	ft_strlcpy(tmp, token->value, idx + 1);
	ft_strlcat(tmp, exit_code, ft_strlen(tmp) + len + 1);
	ft_strlcat(tmp, token->value + idx + 2, malloc_size);
	free(token->value);
	free(exit_code);
	token->value = tmp;
	return (idx + len - 1);
}

int	dollar_with_separator(t_token *token, int idx)
{//$바로뒤에 quote가 오는 경우 quote만 제거하고 확장하지 않는다 //idx는 dollar의 인덱스 //$$나 $?의 경우 여기서 확장하고 처리해준다
	char	*tmp;
	int		len;

	if (token->value[idx + 1] == '\'' || token->value[idx + 1] == '\"') //바로뒤가 quote인 경우 //echo "test$"'$fuck'같은 경우 "~~$"부분은 지워지나 뒤의 부분이 quote제거가 되는지 확인해야한다
		return (dollar_with_quote(token, idx + 1));//반환값은 앞으로 탐색해야하는 idx //idx + 1은 $바로뒤에 오는 quote의 index //""
	else if (token->value[idx + 1] == '$')//$$확장
		return (dollar_with_dollar(token, idx));
	else if (token->value[idx + 1] == '?')//?확장
		return (dollar_with_question_mark(token, idx)); //구현예정
	len = ft_strlen(token->value);
	tmp = malloc(len); //$와 숫자를 제외하고 나머지를 워드로 취급한다
	ft_strlcpy(tmp, token->value, idx + 1); //$직전까지 copy한다
	ft_strlcat(tmp, token->value + idx + 1, len); //$ 이후의 값을 붙여넣는다
	//ft_strlcat(tmp, token->value + idx + 2, len); //$+@ 이후의 값을 붙여넣는다
	free(token->value);
	token->value = tmp;
	printf("dollar_with_separator : %s %d\n",token->value, idx - 1); //$$test ->$test
	return (idx - 1); //반환값은 앞으로 탐색해야하는 idx
}

char	*new_cut_quote(int open, int close, char *target)
{ //open과 close인덱스를 받아서 삭제
	char	*str;
	int		malloc_size;

	malloc_size = ft_strlen(target) - 1; //따옴표 2개를 제거하므로 원래길이에서 1개 감소한다('\0'있으므로)
	str = malloc(ft_strlen(target) - 1);
	ft_strlcpy(str, target, open + 1); //quote전까지 복사한다
	//printf("cut_quote test1 : %s close : %d open : %d\n",str,close, open);
	ft_strlcat(str, target + open + 1, ft_strlen(str) + close - open); //quote이후의 부분을 str에 복사한다//문제발생
	//printf("cut_quote test2 : %s\n",str); //
	ft_strlcat(str, target + close + 1, malloc_size);
	//printf("cut_quote test3 : %s\n",str);
	free(target);
	return (str);
}

char	*del_quote(t_token *token, char *tmp, int start, int *quote_cnt)
{//quote_cnt는 start보다 작은 지점에서 지우는 quote의 개수를 count한다 //start는 $의 index
//모든 quote를 삭제하지 않고 현재와 쌍이 맞는 quote만 삭제한다
	int		idx;
	char	ch;

	idx = -1;
	ch = '\0';
	while ((ch != '\0' && tmp[idx + 1]) || (ch == '\0' && (idx + 1) < start))
	{
		idx++;//tmp = '$USER''$USER', start = 7
		if (token->quote_end < idx && ch == '\0' && (tmp[idx] == '\'' || tmp[idx] == '\"'))
		{//token->quote_end보다 큰 지점에서만 quote를 삭제한다
			ch = tmp[idx];
			token->quote_start = idx;
			if (idx < start)
				(*quote_cnt) += 1;
		}
		else if (ch != '\0' && tmp[idx] == ch) //닫는 quote를 찾은경우 //수정필요있음
		{
			ch = '\0';
			tmp = new_cut_quote(token->quote_start, idx, tmp); //quote를 삭제한다
			token->quote_end = idx - 2; //2개를 지우므로 현재인덱스보다 2개작은 지점을 quote기준으로 삼는다
			if (idx < start) //""$test의 경우 idx = 1, start = 2
				(*quote_cnt) += 1;
			idx = idx - 2; //단어가 2개 줄어들기 때문에 "test"'hi"같은 경우 test파트의 open = 0 close = 5가 된다
			start -= 2; //1105
		}
	}
	printf("after del quote -> copy : %s \n", tmp);
	return (tmp);
}

char	*insert_envp(t_token *token, char *tmp, char *str, int start)
{//start는 붙여넣기할 지점의 idx //$USER -> hgu //echo '$USER'$USER'$USER'
	char	*new; //여기에 tmp와 str을 합친다
	int		new_len;
	int		str_len;

	if (tmp[0] == '\0') //빈 문자열인 경우 문자열을 free해준다 1105
		tmp[0] = '\0';
	// {
	// 	free(tmp);
	// 	return (str);
	// }
	str_len = ft_strlen(str);
	new_len = ft_strlen(tmp) + str_len + 1;
	new = malloc(new_len);
	ft_strlcpy(new, tmp, start + 1); //$앞부분을 넣어준다
	ft_strlcat(new, str, new_len); //확장한 부분을 붙여넣는다
	ft_strlcat(new, tmp + start, new_len); //수정필요
	free(tmp);
	token->expansion_idx = start + str_len; //start는 $의 인덱스, len은 문자열길이 '$USER'$USER -> $USERhgu ->ex_idx = 5 + 3 = 8
	printf("token->expansion_idx : %d\n",token->expansion_idx);
	return (new);
}

int	case_find_in_envp(t_token *token, char *envp, int start, int cut)
{//envp에서 찾은경우 //확장하는 부분과 확장한하는 부분으로 나눈다 //확장안하는 부분에서 quote들을 모두 제거한다 //합친다
	char	*tmp; //확장하지 않는 부분들을 저장할 문자열
	char	*str; //확장결과를 저장할 문자열
	int		quote_cnt; //확장이전에 존재하는 quote의 개수를 센다
	int		len;

	token->expansion_cnt++;
	str = envp + cut - start + 1; //abc=kkk에서 kkk 시작하는부분의 주소
	quote_cnt = 0;
	len = ft_strlen(str);
	tmp = malloc(ft_strlen(token->value)); //넉넉하게 이렇게 만들어둔다 //필요하면 나중에 수정할것
	ft_strlcpy(tmp, token->value, start + 1);//$앞부분까지 복사해둔다
	ft_strlcat(tmp, token->value + cut + 1, ft_strlen(token->value)); //넉넉하게 복사해둔다
	printf("나머지부분들 test : %s\n", tmp);
	tmp = del_quote(token, tmp, start, &quote_cnt); //확장하지 않는 부분인 tmp에서 따옴표를 제거
	free(token->value); //새로만들어야하므로 free해준다
	token->value = insert_envp(token, tmp, str, start - quote_cnt); //insert_envp는 말록한 문자열을 반환한다
	return (start + len - 1 - quote_cnt); //start는 $의 인덱스 len은 확장후 길이
}

void	case_not_find_in_envp(t_token *token, int start, int cut)
{//찾지 못했으면 토큰에서 $~~~부분을 지운다
	int		len;
	int		malloc_size;
	char	*tmp;

	len = ft_strlen(token->value);
	malloc_size = len - (cut - start + 1) + 1;//원래길이 - $~~길이 + 1(NULL) //$abc=123 // start =0, cut = 3
	tmp = malloc(malloc_size);
	ft_strlcpy(tmp, token->value, start + 1);//수정필요 //$나오기 직전까지 원래 문자열을 복사한다
	ft_strlcat(tmp, token->value + cut + 1, malloc_size);//수정필요 //tmp뒤에 원래 $~~ 뒤에 있던 부분을 붙여준다
	token->expansion_fail = token->value; //1106
	//free(token->value); //확장에 실패해도 토큰의 확장실패 임시변수에 저장해둔다
	token->value = tmp;
	return ;
}

int	new_expand(t_token *token, int start, int cut, t_bundle *bundle)
{//3가지 경우가 존재한다 1. $뒤에 영문자, _ 를 제외한게 오는경우 2. envp에서 찾지 못한경우 3. envp에서 찾아서 확장하는경우
	int	idx; //envp탐색에 사용하는 인덱스
	int	after_expansion; //확장후 탐색을 시작해야하는 index

	idx = -1;
	//if (expansion_separator(token->value[idx + 1]) == 1) //$바로뒤가 구분자이면 $와 구분자를 묶어서 지워준다
	if (token->value[start + 1] == '?' || ft_isalpha(token->value[start + 1]) == 0) //$?이거나 $뒤가 알파벳이 아니면
		return (dollar_with_separator(token, start));
	while (bundle->envp[++idx] != NULL) //문자열배열의 마지막원소는 NULL이므로 순회
	{
		printf("envp : %s start : %d cut : %d\n", bundle->envp[idx], start, cut); //05 $hi=user
		printf("token->value : %s envp : %s\n", token->value + start + 1, bundle->envp[idx]);
		if (strncmp(token->value + start + 1, bundle->envp[idx], cut - start) == 0 && bundle->envp[idx][cut - start] == '=') //envp의 명단중 하나와 일치하는 경우
		{//일치하는 경우엔 $hi@@같은 경우이면 현재 토큰이 $hi와 @@로 나뉜다, 만악 $hi가 존재하지 않으면 token을 삭제한다
			printf("find envp\n");
			after_expansion = case_find_in_envp(token, bundle->envp[idx], start, cut); //start는 $의 인덱스, cut은 마지막 문자의 인덱스
			return (after_expansion); //확장 성공하는경우 //앞으로 탐색해야하는 idx를 반환한다
		}
	}// 찾지 못했으면 $~~를 삭제한다
	case_not_find_in_envp(token, start, cut); //$~~부분을 삭제한다 //오류발생 hi$USER
	return (start - 1);
}

int	check_dollar(t_token *token, int idx, int *flag, t_bundle *bundle)
{
	char	*str;
	int		cut;

	str = token->value;
	if (*flag >= 0 && str[idx] == '$')//현재 인덱스가 $이면 확장을 수행한다
	{
		if (str[idx + 1] == '\0') //$하나로 끝나면 바로 종료
			return (idx);
		else if (*flag == 0 && str[idx + 1] == '\"') //"$"같은 경우 확장과 quote삭제를 통해 없어지게 되는데 "밖으로 나오는 효과를 주기위해 flag를 1로 초기화한다
			*flag = 1;
		cut = cut_env_idx(token->value, idx); //환경변수 마지막 문자의 주소를 반환하도록 한다
		idx = new_expand(token, idx, cut, bundle);
	}
	return (idx);
}

void	check_expansion_cnt(t_token *token)
{
	if (token->expansion_cnt == 0)
		delete_quote(token);
	else
		delete_quote_after_expansion(token);
}

t_token	*expansion_main(t_token *head, t_bundle *bundle)
{
	t_token *tmp;
	int		idx;
	int		flag; //flag == 1이면 큰따옴표나 따옴표밖에 있는 상태 -> 확장가능, flag == -1이면 작은 따옴표 안인 상태 -> 확장불가능

	if (head == NULL) //문법단계나 tokenize단에서 오류가 나면 NULL포인터로 들어온다
		return (NULL);
	tmp = head->next; //token의 head는 빈문자를 가지고 있음
	while (tmp != NULL) //토큰을 마지막까지 탐색한다
	{
		idx = -1;
		printf("before expansion token : %s\n",tmp->value);
		flag = 1; //flag : 1 이면 따옴표밖 0이면 큰 따옴표안, -1이면 작은 따옴표안1
		while (tmp->value[++idx] != '\0') //토큰의 value 문자열을 탐색한다
		{
			if (flag == 1 && tmp->value[idx] == '\'') //작은 따옴표는 $로 확장이 불가능하므로 flag를 따로 준다
				flag = -1; //작은 따옴표안에 있는 상태
			else if (flag == 1 && tmp->value[idx] == '\"') //큰 따옴표만나면 flag 0으로
				flag = 0; //큰 따옴표 안에 있는 경우
			else if (flag == 0 && tmp->value[idx] == '\"') //큰따옴표종료
				flag = 1; //따옴표 밖에 있는 경우
			else if (flag == -1 && tmp->value[idx] == '\'')//작은 따옴표 종료
				flag = 1; //따옴표 밖에 있는 경우
			idx = check_dollar(tmp, idx, &flag, bundle); //check_dollar함수는 tmp->value[idx] == '$'이면 확장을 수행하고 마지막 지점의 idx반환
			printf("idx : %d value : %s\n",idx, tmp->value);		
		}
		check_expansion_cnt(tmp); //확장여부를 체크하고 확장되지 않았으면 quote제거하는 작업을 수행한다
		printf("after expansion token : %s\n",tmp->value);
		tmp = tmp->next; //만약 현재 token에서 $를 발견하면 확장 -> 
	}
	return (delete_empty_token(head)); //토큰을 전부 순회하면서 빈 문자열이 있는 토큰들을 제거해준다
}
