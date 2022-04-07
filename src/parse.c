#include "../includes/minishell.h"

void print_token(char **tokens) // erase when no longer needed
{
	int i;

	i = 0;
	while (tokens[i] != NULL)
	{
		printf("%s,", tokens[i]);
		i++;
	}
	printf("\n");
}

void print_cmd_blocks(t_list *cmd_blocks) // erase when no longer needed
{
	t_list	*temp_block;

	temp_block = cmd_blocks;
	while (temp_block != NULL)
	{
		printf("index: %d\n", temp_block->index_cmd);
		if (temp_block->cmd != NULL)
		{
			printf("cmd: ");
			print_token(temp_block->cmd);
		}
		if (temp_block->redirect != NULL)
		{
			while (temp_block->redirect != NULL)
			{
				printf("op & file: ");
				printf("%s %s, ", (temp_block->redirect)->op, (temp_block->redirect)->file);
//				printf("next redirect: %p\n", (void *)(temp_block->redirect)->next);
				temp_block->redirect = (temp_block->redirect)->next;
			}
			printf("\n");
		}
		printf("\n");
		temp_block = temp_block->next;
	}
}

t_list	*parse_line(char *str, t_data *data)
{
	char	**tokens;
	t_list	*cmd_blocks;

	tokens = ft_split_minishell(str, ' ');
//	print_token(tokens); // remove 
	if (tokens != NULL)
	{
		if (check_syntax(tokens) == 1)
			error_syntax(&tokens);
		else 
		{
//			printf("syntax correct\n"); // remove once testing is no longer required
			tokens = expansion(tokens, data);
			print_token(tokens);  // remove

			cmd_blocks = set_cmd_blocks(tokens);
			free_double(&tokens);
			print_cmd_blocks(cmd_blocks);
	
			return (cmd_blocks);
		}
	}
	return (NULL);
}