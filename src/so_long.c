/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   so_long.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ashobajo <ashobajo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/20 20:27:00 by ashobajo          #+#    #+#             */
/*   Updated: 2024/09/21 16:02:48 by ashobajo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "so_long.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int	isber(char *file)
{
	int				len;

	len = ft_strlen(file);
	if (file == 0)
		return (0);
	if (len < 5)
		return (0);
	if (ft_strcmp(file + len - 4, ".ber") != 0)
		return (0);
	return (1);
}

int	main(int argc, char **argv)
{
	t_base	*base;

	if (argc != 2)
	{
		printf("\033[1;31mERROR: usage: ./solong map.ber\033[0m\n");
		return (1);
	}
	else if (ft_strlen(argv[1]) < 5)
	{
		printf("\033[1;31mERROR: invalid map.ber\033[0m\n");
		return (1);
	}
	printf("\033[1;32mWelcome to Solong by AbdulAzeez\033[0m\n");

	base = base_init(argv[1]);
	create_base(base);
	mlx_hook(base->mlx_win, 2, 1L << 0, key_press, base);
	mlx_hook(base->mlx_win, 3, 1L << 1, key_release, base);
	mlx_hook(base->mlx_win, 17, 1L << 17, destroy_game, base);
	mlx_loop(base->mlx);
    return (0);
}

t_base *base_init(char *filename)
{
	t_base *base;
	
	base = (t_base *)malloc(sizeof(t_base));
	if (base == 0)
		base_destroy(0, "base_init(): malloc()", errno);
	base->game = 0;
	base->mlx = 0;
	base->mlx_win = 0;
	base->mlx_img = 0;
	base->player = 0;
	base->exit = 0;
	base->coll = 0;
	base->wall = 0;
	base->ground = 0;
	game_init(base, filename);
	render_game(base);
	return(base);
}

void game_init(t_base *base, char *filename)
{
	base->game = (t_game *)malloc(sizeof(t_game));
	if (base->game == 0)
		base_destroy(base, "game_init(): malloc()", errno);
	base->game->map = 0;
	base->game->coll = 0;
	base->game->count_coll = 0;
	base->game->count_exit = 0;
	base->game->count_player = 0;
	base->game->player_move = 0;
	base->game->player_coll = 0;
	base->game->player_up = 0;
	base->game->player_down = 0;
	base->game->player_left = 0;
	base->game->player_right = 0;
	map_init(base, filename);

}


void map_init(t_base *base, char *filename)
{
	int fd;
	char *file;
	char buffer[1024 + 1];
	int ret;
	char *tmp;
	
	file = ft_calloc(1, 1);
	if (file == 0)
	{
		base_destroy(base, "map_init(): ft_calloc()", errno);
	}
	fd = open(filename, O_RDONLY);
	if(fd == -1)
	{
		printf("\033[1;31mERROR: invalid map\033[0m\n");
		base_destroy(base, filename, errno);
	}

	ret = 1;
	
	while (ret != 0)
	{
		ret = read(fd, buffer, 1024);
		if (ret == -1)
		{
			free(file);
			close(fd);
			base_destroy(base, "map_init(): read()", errno);
		}
		
		else
		{
			buffer[ret] = 0;	
			tmp = ft_strjoin(file, buffer);
			free(file);
			file = tmp;
			if (*file == 0)
			{
				close(fd);
				base_destroy(base, "map_init(): ft_strjoin()", errno);
			}
		}

	}

	close(fd);

	read_map(base, file);
	free(file);
}

void	map_width(t_base *base, char *file)
{
	base->game->width = 0;
	while (file[base->game->width] && file[base->game->width] != '\n')
		base->game->width++;
}

void	map_height(t_base *base, char *file)
{
	int				i;
	int				j;

	base->game->height = 1;
	i = base->game->width + 1;
	while (file[i] != 0)
	{
		j = 0;
		while (file[i + j] != 0 && file[i + j] != '\n')
			j++;
		if (base->game->width != j)
		{
			free(file);
			base_destroy(base, "map format is invalid", 0);
		}
		i += base->game->width + 1;
		base->game->height++;
	}
}


void read_map(t_base *base, char *file)
{
	map_width(base, file);
	map_height(base, file);
	map_isvalid(base, file);
	base->game->coll = (t_axis *)malloc(sizeof(t_axis) * base->game->count_coll);
	if (base->game->coll == 0)
	{
		free(file);
		base_destroy(base, "base_parsing(): malloc()", errno);
	}
	base->game->map = (int **)malloc(sizeof(int *)* base->game->height);
	if (base->game->map == 0)
	{
		free(file);
		base_destroy(base, "map_parsing(): malloc()", errno);
	}
	draw_map(base, file);
}

void draw_map(t_base *base, char *file)
{
	int i, j, k;
	int	obj;
	
	j = 0;
	k = 0;
	obj = 0;

	while (j < base->game->height)
	{
		base->game->map[j] = (int *)malloc(sizeof(int ) * base->game->width);
		i = 0;
		while (i < base->game->width)
		{
			get_axis(base, file, k, &obj);
			base->game->map[j][i] = file[k] - 48;
			k++;
			i++;
		}
		k++;
		j++;
	}
}

void get_axis(t_base *base, char *file, int k, int * obj)
{
	if (file[k] == 'P')
	{
		base->game->player.x = k % (base->game->width + 1);
		base->game->player.y = k / (base->game->width + 1);
		file[k] = '0';
	}

	if (file[k] == 'E')
	{
		base->game->exit.x = k % (base->game->width + 1);
		base->game->exit.y = k / (base->game->width + 1);
		file[k] = '0';
	}
	
	if (file[k] == 'C')
	{
		base->game->coll[*obj].x = k % (base->game->width + 1);
		base->game->coll[*obj].y = k / (base->game->width + 1);
		file[k] = '0';
		(*obj)++;
	}
}

void	base_destroy(t_base *base, char *errmsg, int errnum)
{
	if (base != 0)
	{
		if (base->ground != 0)
			mlx_destroy_image(base->mlx, base->ground);
		if (base->wall != 0)
			mlx_destroy_image(base->mlx, base->wall);
		if (base->coll != 0)
			mlx_destroy_image(base->mlx, base->coll);
		if (base->exit != 0)
			mlx_destroy_image(base->mlx, base->exit);
		if (base->player != 0)
			mlx_destroy_image(base->mlx, base->player);
		if (base->mlx_img != 0)
			mlx_destroy_image(base->mlx, base->mlx_img);
		if (base->mlx_win != 0)
			mlx_destroy_window(base->mlx, base->mlx_win);
		if (base->mlx != 0)
			mlx_destroy_display(base->mlx);
		if (base->game != 0)
			game_destroy(base->game);
		free(base);
	}
	die(errmsg, errnum);
}

void	game_destroy(t_game *game)
{
	int				i;

	if (game != 0)
	{
		if (game->coll != 0)
			free(game->coll);
		if (game->map != 0)
		{
			i = 0;
			while (i < game->height)
				free(game->map[i++]);
			free(game->map);
		}
		free(game);
	}
}

unsigned int	mlx_get_pixel(t_img *img, int x, int y)
{
	return (*(unsigned int *)
		(img->data + (x * img->bpp / 8 + y * img->size_line)));
}

void	mlx_draw_pixel(t_img *mlx_img, int x, int y, int color)
{
	char			*target;

	target = mlx_img->data + (x * mlx_img->bpp / 8 + y * mlx_img->size_line);
	*(unsigned int *)target = color;
}

unsigned int	mlx_rgb_to_int(int o, int r, int g, int b)
{
	return (o << 24 | r << 16 | g << 8 | b);
}

static void	draw_square(t_base *base, t_img *img, int x, int y)
{
	unsigned int	color;
	int				i;
	int				j;

	j = 0;
	while (j < 40)
	{
		i = 0;
		while (i < 40)
		{
			color = mlx_get_pixel(img, i, j);
			if (color != mlx_rgb_to_int(0, 255, 255, 255))
				mlx_draw_pixel(base->mlx_img, x + i, y + j, color);
			i++;
		}
		j++;
	}
}

static void	draw_env(t_base *base, int i, int j)
{
	int				k;

	if (base->game->exit.x == i && base->game->exit.y == j)
		draw_square(base, base->exit, i * 40, j * 40);
	k = -1;
	while (++k < base->game->count_coll)
		if (base->game->coll[k].x == i && base->game->coll[k].y == j)
			draw_square(base, base->coll, i * 40, j * 40);
	if (base->game->player.x == i && base->game->player.y == j)
		draw_square(base, base->player, i * 40, j * 40);
}

static void	now_draw_map(t_base *base)
{
	int				i;
	int				j;

	j = 0;
	while (j < base->game->height)
	{
		i = 0;
		while (i < base->game->width)
		{
			if (base->game->map[j][i] == 1)
				draw_square(base, base->wall, i * 40, j * 40);
			else
				draw_square(base, base->ground, i * 40, j * 40);
			draw_env(base, i, j);
			i++;
		}
		j++;
	}
}

void	create_base(t_base *base)
{
	now_draw_map(base);
	mlx_put_image_to_window(base->mlx, base->mlx_win, base->mlx_img, 0, 0);
}

