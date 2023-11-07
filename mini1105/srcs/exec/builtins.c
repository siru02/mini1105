/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hyuim <hyuim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/11 21:11:37 by hyuim             #+#    #+#             */
/*   Updated: 2023/11/06 19:16:10 by hyuim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

int	echo(char **cmd_argv)
{
	int	n_flag;
	int	idx;

	n_flag = 0;
	if (cmd_argv[1] && ft_strlen(cmd_argv[1]) == 2 && ft_strcmp(cmd_argv[1], "-n") == 0)
		n_flag = 1;
	idx = n_flag;
	while (cmd_argv[++idx])
	{
		if (write(STDOUT_FILENO, cmd_argv[idx], ft_strlen(cmd_argv[idx])) == -1 || write(STDOUT_FILENO, " ", 1) == -1)
		{
			perror(NULL);
			return (1);
		}
	}
		if (!n_flag)
		{
			if (write(STDOUT_FILENO, "\n", 1) == -1)
			{
				perror(NULL);
				return (1);
			}
		}
	return (0);
}

int	cd(t_bundle *bundle, char **cmd_argv)
{
	char	*dest;

	if (!cmd_argv[1])
	{
		dest = getenv_minishell(bundle, "HOME=");
		if (!dest)
		{
			write(STDERR_FILENO, "cd: HOME not set\n", 17);
			return (1);
		}
		dest += 5;
	}
	else
		dest = cmd_argv[1];
	if (chdir(dest) == -1)
	{	
		if (!access(dest, F_OK))
		{
			write(STDERR_FILENO, "cd: ", 4);
			write(STDERR_FILENO, dest, ft_strlen(dest));
			write(STDERR_FILENO, ": Not a directory\n", 18);
			return (1);
		}
		write(STDERR_FILENO, "cd: ", 4);
		write(STDERR_FILENO, dest, ft_strlen(dest));
		perror(" ");
		return (1);
	}
	return (0);
}

char	*getenv_minishell(t_bundle *bundle, char *var_name)
{
	int	idx;
	size_t	var_len;

	idx = -1;
	var_len = ft_strlen(var_name);
	while (bundle->envp[++idx])
		if (ft_strlen(bundle->envp[idx]) >= var_len && ft_strncmp(bundle->envp[idx], var_name, var_len) == 0)
			break;
	return (bundle->envp[idx]);
}

int	pwd()
{
	char	*cwd;

	cwd = (char *)malloc(sizeof(char) * PATH_MAX);
	if (!cwd)
	{
		perror(NULL);
		return (1);
	}
	if (!getcwd(cwd, PATH_MAX))
	{
		free(cwd);
		if (write(STDERR_FILENO, "current directory would be deleted.\n", 36) == -1)
			ft_error("", 1);
		return (1);
	}
	if (write(STDERR_FILENO, cwd, ft_strlen(cwd))== -1 || write(STDERR_FILENO, "\n", 1) == -1)
		ft_error("", 1);
	free(cwd);
	return (0);
}

int	print_declare_env(char **str_arr)
{
	int i = -1;
	while (str_arr[++i])
	{
		if (write(2, "declare -x ", 11) == -1)
			return (1);
		if (write(STDOUT_FILENO, str_arr[i], ft_strlen(str_arr[i])) == -1)
			return (1);
		if (write(STDOUT_FILENO, "\n", 1) == -1)
			return (1);
	}
	return (0);
}

int	export(t_bundle *bundle, char **cmd_argv)
{
	int	idx;
	int	err_flag;

	idx = -1;
	while (cmd_argv[++idx])
		;
	if (idx == 1)
		return (print_declare_env(bundle->envp));
	idx = 0;
	err_flag = 0;
	while (cmd_argv[++idx])
	{
		if (check_exeception(cmd_argv[idx]) == -1)
		{
			printf("export here0\n");
			err_flag = 1;
			if (write(STDERR_FILENO, "export: '", 9) == -1
				|| write(STDERR_FILENO, cmd_argv[idx], ft_strlen(cmd_argv[idx])) == -1
				|| write(STDERR_FILENO, "': not a valid identifier\n", 26) == -1) // re-code err msg
				ft_error(WRITE_ERRMSG, 1234);
		}
		else if (ft_strlen(cmd_argv[idx]) >= 2 && cmd_argv[idx][0] == '_' && cmd_argv[idx][1] == '=')
			continue ;
		else
			append_new_env_var(bundle, -1, cmd_argv[idx]);
	}
	return (err_flag);
}

void	append_new_env_var(t_bundle *bundle, int ret_envp_idx, char *new_str)
{
	char	**ret_envp;

	if (!ft_strchr(new_str, '='))
		return ;
	ret_envp = (char **)malloc(sizeof(char *) * (bundle->envp_len + 2));
	if (!ret_envp)
		ft_error(MALLOC_ERRMSG, 1234);
	ret_envp[bundle->envp_len + 1] = NULL;
	while (++ret_envp_idx < bundle->envp_len)
		ret_envp[ret_envp_idx] = bundle->envp[ret_envp_idx];
	ret_envp[ret_envp_idx] = ft_strdup(new_str);
	if (!ret_envp[ret_envp_idx])
		ft_error(MALLOC_ERRMSG, 1234);
	free(bundle->envp);
	bundle->envp = ret_envp;
	bundle->envp_len++;
}

int	check_exeception(char *inp)
{
	int	idx;

	if (!((inp[0] >= 'a' && inp[0] <= 'z') || (inp[0] >= 'A' && inp[0] <= 'Z') || inp[0] == '_'))
		return (-1);
	idx = -1;
	while (inp[++idx] && inp[idx] != '=')
		if (!((inp[idx] >= 'a' && inp[idx] <= 'z')
			|| (inp[idx] >= 'A' && inp[idx] <= 'Z')
			|| inp[idx] == '_'
			|| (inp[idx] >= '0' && inp[idx] <= '9')))
			return (-1);
	return (0);
}

int	unset(t_bundle *bundle, char **cmd_argv)
{
	int	idx;
	int	err_flag;
	int	found_idx;

	idx = 0;
	err_flag = 0;
	while (cmd_argv[++idx])
	{
		if (check_exeception(cmd_argv[idx]) == -1)
			err_flag = 1;
		else if (ft_strlen(cmd_argv[idx]) >= 1 && cmd_argv[idx][0] == '_')
			continue ;
		else
		{
			found_idx = is_in_envp(bundle, cmd_argv[idx]);
			if (found_idx != -1)
				rm_inp_env_var(bundle, found_idx);
		}
	}
	return (err_flag);
}

//int	check_exeception_unset(char *inp)
//{
//	int	idx;

//	if (!((inp[0] >= 'a' && inp[0] <= 'z') || inp[0] == '_'))
//		return (-1);
//	idx = -1;
//	while (inp[++idx] && inp[idx] != '=')
//		if (!((inp[idx] >= 'a' && inp[idx] <= 'z') || inp[idx] == '_' || (inp[idx] >= '0' && inp[idx] <= '9')))
//			return (-1);
//	return (0);
//}

int	is_in_envp(t_bundle *bundle, char *inp_str)
{
	int	idx;
	size_t	inp_len;

	idx = -1;
	inp_len = ft_strlen(inp_str);
	while (bundle->envp[++idx])
	{
		if (ft_env_var_len(bundle->envp[idx]) == inp_len && !ft_strncmp(bundle->envp[idx], inp_str, inp_len))
			return (idx);
	}
	return (-1);
}

size_t	ft_env_var_len(const char *s)
{
	size_t	idx;

	idx = 0;
	while (s[idx] && s[idx] != '=')
		idx++;
	return (idx);
}

void	rm_inp_env_var(t_bundle *bundle, int found_idx)
{
	char	**ret_envp;
	int		idx;
	int		ret_envp_idx;

	ret_envp = (char **)malloc(sizeof(char *) * (bundle->envp_len));
	if (!ret_envp)
		ft_error(MALLOC_ERRMSG, 1234);
	idx = -1;
	ret_envp_idx = -1;
	while (++idx < bundle->envp_len)
	{
		if (idx == found_idx)
			continue ;
		else
			++ret_envp_idx;
		ret_envp[ret_envp_idx] = bundle->envp[idx];
	}
	free(bundle->envp[found_idx]);
	free(bundle->envp);
	bundle->envp = ret_envp;
	bundle->envp_len--;
}

int	env(t_bundle *bundle)
{
	int i = -1;
	while (++i < bundle->envp_len)
	{
		if (write(STDOUT_FILENO, bundle->envp[i], ft_strlen(bundle->envp[i])) == -1)
			return (1);
		if (write(STDOUT_FILENO, "\n", 1) == -1)
			return (1);
	}
	return (0);
}

void	ft_exit(char **cmd_argv)
{
	int	atoi_ret;
	int	cmd_argv_size;

	cmd_argv_size = -1;
	atoi_ret = 0;
	while (cmd_argv[++cmd_argv_size])
		;
	if (cmd_argv_size > 2)
	{
		if (write(2, "exit: too many arguments\n", 25) == -1)
			ft_error(WRITE_ERRMSG, 1234);
		atoi_ret = 1;
	}
	else if (cmd_argv_size == 2)
	{
		atoi_ret = ft_atoi_exit(cmd_argv[1]);
		if (atoi_ret == -1)
		{
			if (write(2, "exit: numeric argument required\n", 33) == -1)
				ft_error(WRITE_ERRMSG, 1234);
			atoi_ret = 1;
		}
	}
	exit(atoi_ret);
}

int	ft_atoi_exit(const char *str)
{
	int		sign;
	unsigned long long	ret;

	ret = 0;
	sign = 1;
	if (*str == 0)
		return (-1);
	if (*str == '-' || *str == '+')
	{
		if (*str == '-')
			sign *= -1;
		str++;
	}
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return (-1);
		ret = ret * 10 + (long)(*str - '0');
		str++;
	}
	ret = sign * ret;
	return (ret % 256);
}

