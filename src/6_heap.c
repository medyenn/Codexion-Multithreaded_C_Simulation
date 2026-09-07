/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_heap.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:25:31 by mennih            #+#    #+#             */
/*   Updated: 2026/09/06 18:29:56 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	req_less(const t_request *a, const t_request *b)
{
	if (a->priority != b->priority)
		return (a->priority < b->priority);
	return (a->coder_id > b->coder_id);
}

void	heap_swap(t_request *heap, int i, int j)
{
	t_request	tmp;

	tmp = heap[i];
	heap[i] = heap[j];
	heap[j] = tmp;
}

void	heap_sift_up(t_request *heap, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (req_less(&heap[idx], &heap[parent]))
		{
			heap_swap(heap, idx, parent);
			idx = parent;
		}
		else
			break ;
	}
}

void	heap_sift_down(t_request *heap, int size, int idx)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		smallest = idx;
		if (left < size && req_less(&heap[left], &heap[smallest]))
			smallest = left;
		if (right < size && req_less(&heap[right], &heap[smallest]))
			smallest = right;
		if (smallest == idx)
			break ;
		heap_swap(heap, idx, smallest);
		idx = smallest;
	}
}

int	heap_push(t_sim *sim, t_request req)
{
	if (sim->heap_size >= sim->heap_cap)
		return (-1);
	sim->heap[sim->heap_size] = req;
	heap_sift_up(sim->heap, sim->heap_size);
	sim->heap_size++;
	return (0);
}
