/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_coder.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:24:30 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 02:05:46 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	take_dongles(t_coder *c)
{
	if (sim_is_stopped(c->sim))
		return (false);
	c->ticket = get_time_ms();
	dongle_request(c->sim, c);
	if (sim_is_stopped(c->sim))
		return (false);
	log_event(c->sim, c->id, "has taken a dongle");
	log_event(c->sim, c->id, "has taken a dongle");
	return (true);
}

static void	run_compile_phase(t_coder *c, t_sim *sim)
{
	pthread_mutex_lock(&c->cond_mutex);
	c->last_compile_start = get_time_ms();
	pthread_mutex_unlock(&c->cond_mutex);
	log_event(sim, c->id, "is compiling");
	msleep(sim->time_to_compile);
	put_dongles(c);
}

static bool	run_debug_refactor(t_coder *c, t_sim *sim)
{
	if (sim_is_stopped(sim))
		return (false);
	log_event(sim, c->id, "is debugging");
	msleep(sim->time_to_debug);
	if (sim_is_stopped(sim))
		return (false);
	log_event(sim, c->id, "is refactoring");
	msleep(sim->time_to_refactor);
	return (true);
}

static bool	coder_cycle(t_coder *c, t_sim *sim)
{
	pthread_mutex_lock(&c->cond_mutex);
	if (c->compile_count >= sim->compiles_required)
	{
		pthread_mutex_unlock(&c->cond_mutex);
		return (false);
	}
	pthread_mutex_unlock(&c->cond_mutex);
	if (!take_dongles(c))
		return (false);
	run_compile_phase(c, sim);
	if (!run_debug_refactor(c, sim))
		return (false);
	pthread_mutex_lock(&c->cond_mutex);
	c->compile_count++;
	pthread_mutex_unlock(&c->cond_mutex);
	return (true);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;
	t_sim	*sim;

	c = (t_coder *)arg;
	sim = c->sim;
	while (!sim_is_stopped(sim))
	{
		if (!coder_cycle(c, sim))
			break ;
	}
	return (NULL);
}
