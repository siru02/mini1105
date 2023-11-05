/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_bundle.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hyuim <hyuim@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/01 22:31:46 by hyuim             #+#    #+#             */
/*   Updated: 2023/11/01 23:27:55 by hyuim            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../headers/minishell.h"

void	free_bundle(t_bundle *bundle)
{
	int	idx;

	idx = -1;
	while (bundle->envp[++idx])
		free(bundle->envp[idx]);
	free(bundle->envp);
}