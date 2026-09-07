/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   5_dongle.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:25:16 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 14:01:48 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	dongle_init(t_dongle *d, int id)
{
	d->id = id;
	d->in_use = false;
	d->release_time = 0;
}

bool	dongle_ready(t_dongle *d, long long cooldown_ms)
{
	long long	now;

	if (d->in_use)
		return (false);
	if (d->release_time == 0)
		return (true);
	now = get_time_ms();
	return (now >= d->release_time + cooldown_ms);
}

void	dispatch(t_sim *sim)
{
	t_request	req;
	t_coder		*c;
	int			pending;

	pending = 0;
	while (sim->heap_size > 0)
	{
		req = sim->heap[0];
		heap_remove(sim, 0);
		c = &sim->coders[req.coder_id - 1];
		if (c->left != c->right
			&& dongle_ready(c->left, sim->dongle_cooldown)
			&& dongle_ready(c->right, sim->dongle_cooldown))
			grant_request(c);
		else
			sim->pending[pending++] = req;
	}
	while (pending > 0)
		heap_push(sim, sim->pending[--pending]);
}

void	dongle_request(t_sim *sim, t_coder *c)
{
	t_request	req;
	int			idx;

	req.coder_id = c->id;
	req.priority = c->ticket;
	if (sim->scheduler == POLICY_EDF)
		req.priority = c->last_compile_start + sim->time_to_burnout;
	pthread_mutex_lock(&c->cond_mutex);
	c->granted = false;
	pthread_mutex_unlock(&c->cond_mutex);
	pthread_mutex_lock(&sim->arb_mutex);
	heap_push(sim, req);
	dispatch(sim);
	pthread_mutex_unlock(&sim->arb_mutex);
	if (!coder_wait_grant(sim, c))
	{
		pthread_mutex_lock(&sim->arb_mutex);
		idx = heap_find(sim, c->id);
		if (idx != -1)
			heap_remove(sim, idx);
		pthread_mutex_unlock(&sim->arb_mutex);
	}
}

void	dongle_release(t_sim *sim, t_coder *c)
{
	pthread_mutex_lock(&sim->arb_mutex);
	c->left->in_use = false;
	c->left->release_time = get_time_ms();
	if (c->right != c->left)
	{
		c->right->in_use = false;
		c->right->release_time = get_time_ms();
	}
	dispatch(sim);
	pthread_mutex_unlock(&sim->arb_mutex);
}
