/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_simulator.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:26:14 by mennih            #+#    #+#             */
/*   Updated: 2026/09/06 21:09:10 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	sim_is_stopped(t_sim *sim)
{
	bool	stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stopped;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

void	sim_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->stop_mutex);
	sim->stopped = true;
	pthread_mutex_unlock(&sim->stop_mutex);
}

static int	start_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n)
	{
		if (pthread_create(&sim->threads[i], NULL,
				coder_routine, &sim->coders[i]) != 0)
		{
			fprintf(stderr, "Error: pthread_create coder %d.\n", i + 1);
			sim_stop(sim);
			while (--i >= 0)
				pthread_join(sim->threads[i], NULL);
			return (-1);
		}
		i++;
	}
	return (0);
}

int	sim_run(t_sim *sim)
{
	int	i;

	if (start_coders(sim) != 0)
		return (-1);
	if (pthread_create(&sim->monitor_thread, NULL, monitor_routine, sim) != 0)
	{
		fprintf(stderr, "Error: pthread_create monitor.\n");
		sim_stop(sim);
		i = 0;
		while (i < sim->n)
			pthread_join(sim->threads[i++], NULL);
		return (-1);
	}
	i = 0;
	while (i < sim->n)
		pthread_join(sim->threads[i++], NULL);
	pthread_join(sim->monitor_thread, NULL);
	return (0);
}
