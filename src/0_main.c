/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_main.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mennih < mennih@student.1337.ma>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 14:25:54 by mennih            #+#    #+#             */
/*   Updated: 2026/09/07 01:59:41 by mennih           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_sim	sim;
	int		ret;

	memset(&sim, 0, sizeof(t_sim));
	if (parse_args(argc, argv, &sim) != 0)
		return (1);
	if (sim.compiles_required == 0)
		return (0);
	if (sim_init(&sim) != 0)
	{
		fprintf(stderr, "Error: initialisation failed.\n");
		return (1);
	}
	ret = sim_run(&sim);
	sim_cleanup(&sim);
	if (ret != 0)
		return (1);
	return (0);
}
