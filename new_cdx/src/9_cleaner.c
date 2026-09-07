/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   9_cleaner.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:24:08 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 14:00:30 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	free_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n)
	{
		pthread_cond_destroy(&sim->coders[i].cond);
		pthread_mutex_destroy(&sim->coders[i].cond_mutex);
		i++;
	}
	free(sim->coders);
	sim->coders = NULL;
}

void	sim_cleanup(t_sim *sim)
{
	if (sim->threads)
	{
		free(sim->threads);
		sim->threads = NULL;
	}
	if (sim->coders)
		free_coders(sim);
	if (sim->dongles)
		free_dongle_arrays(sim);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->arb_mutex);
}
