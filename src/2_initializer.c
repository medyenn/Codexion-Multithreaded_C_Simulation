/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   2_initializer.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:25:48 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 14:00:13 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_mutexes(t_sim *sim)
{
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (-1);
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		return (-1);
	}
	if (pthread_mutex_init(&sim->arb_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		pthread_mutex_destroy(&sim->stop_mutex);
		return (-1);
	}
	return (0);
}

static int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = (t_dongle *)malloc((size_t)sim->n * sizeof(t_dongle));
	sim->heap = (t_request *)malloc((size_t)sim->n * sizeof(t_request));
	sim->pending = (t_request *)malloc((size_t)sim->n * sizeof(t_request));
	if (!sim->dongles || !sim->heap || !sim->pending)
	{
		free_dongle_arrays(sim);
		return (-1);
	}
	sim->heap_size = 0;
	sim->heap_cap = sim->n;
	i = 0;
	while (i < sim->n)
	{
		dongle_init(&sim->dongles[i], i);
		i++;
	}
	return (0);
}

static int	init_coder(t_sim *sim, int i)
{
	sim->coders[i].id = i + 1;
	sim->coders[i].left = &sim->dongles[i];
	sim->coders[i].right = &sim->dongles[(i + 1) % sim->n];
	sim->coders[i].last_compile_start = sim->start_time_ms;
	sim->coders[i].compile_count = 0;
	sim->coders[i].ticket = 0;
	sim->coders[i].granted = false;
	sim->coders[i].burned_out = false;
	sim->coders[i].sim = sim;
	if (pthread_cond_init(&sim->coders[i].cond, NULL) != 0)
		return (-1);
	if (pthread_mutex_init(&sim->coders[i].cond_mutex, NULL) != 0)
	{
		pthread_cond_destroy(&sim->coders[i].cond);
		return (-1);
	}
	return (0);
}

static int	init_coders_loop(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->n)
	{
		if (init_coder(sim, i) != 0)
		{
			while (--i >= 0)
			{
				pthread_cond_destroy(&sim->coders[i].cond);
				pthread_mutex_destroy(&sim->coders[i].cond_mutex);
			}
			free(sim->coders);
			sim->coders = NULL;
			return (-1);
		}
		i++;
	}
	return (0);
}

int	sim_init(t_sim *sim)
{
	sim->stopped = false;
	sim->start_time_ms = get_time_ms();
	if (init_mutexes(sim) != 0)
		return (-1);
	if (init_dongles(sim) != 0)
	{
		pthread_mutex_destroy(&sim->log_mutex);
		pthread_mutex_destroy(&sim->stop_mutex);
		pthread_mutex_destroy(&sim->arb_mutex);
		return (-1);
	}
	sim->coders = (t_coder *)malloc((size_t)sim->n * sizeof(t_coder));
	if (!sim->coders || init_coders_loop(sim) != 0)
	{
		sim_cleanup(sim);
		return (-1);
	}
	sim->threads = (pthread_t *)malloc((size_t)sim->n * sizeof(pthread_t));
	if (!sim->threads)
	{
		sim_cleanup(sim);
		return (-1);
	}
	return (0);
}
