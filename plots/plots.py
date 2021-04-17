import ijson
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import time

# Apply custom style sheet globally
plt.style.use('custom-dark')


def get_name(json_filename, player_number):
    with open(json_filename, 'rb') as f:
        object = ijson.items(f, 'stats.player' + str(player_number) + '.name')
        for name in object:
            return name


# Get numpy array from original json stats-file
def get_np_array(json_filename, player_number, category):
    with open(json_filename, 'rb') as f:
        object = ijson.items(f, 'stats.player' + str(player_number) + '.' + category)
        for numbers in object:
            return np.array(numbers)


# Append numpy arrays from multiple saves 
def combine_saves(json_filename_list, player_number, category):
    combined_numbers = np.array([])
    for json_filename in json_filename_list:
        combined_numbers.append(get_np_array(json_filename, player_number, category))
    #TODO: clean-up
    return combined_numbers


# Iterate over all categorys 
def save_np_arrays(json_filename, category):
    pass


def plot_stats_category(json_filename, category, intervall=None, n_xticks=None):
    fig, ax = plt.subplots() 

    # Plot curves
    for i in range(2):  # TODO: Switch range to numpy array size
        numbers = get_np_array(json_filename, i, category)
        print('recorded values: ', numbers.size)
        if np.any(numbers):  # TODO: Switch the check whether a spot is taken to the player name
            if intervall:
                ax.plot(numbers[intervall[0]:intervall[1]], label='Player'+str(i)) 
            else:
                ax.plot(numbers, label='Player'+str(i)) 

    # Formatting
    formatter = matplotlib.ticker.FuncFormatter(lambda datapoint, x: time.strftime('%H:%M', time.gmtime(datapoint / 1.3333)))
    ax.xaxis.set_major_formatter(formatter)
    if n_xticks:
        ax.xaxis.set_major_locator(plt.MaxNLocator(n_xticks))
    ax.legend()
    ax.set_title(category)
    ax.set_ylim(bottom=0)
    ax.set_xlim(left=0)
    plt.show()


def stack_chart():
    x=range(1,6)
    y1=[1,4,6,8,9]
    y2=[2,2,7,10,12]
    y3=[2,8,5,10,6]

    # Basic stacked area chart.
    plt.stackplot(x,y1, y2, y3, labels=['A','B','C'])
    plt.legend()
    plt.show()


if __name__ == '__main__':
    # Testrecording, setmap, 2 amazons on gangbang map
    filename = 'example/s3-2021-04-15-22-39-46.json'
    intervall=(350, 5250)
    n_xticks=7
    plot_stats_category(filename, category='buildings', intervall=intervall, n_xticks=n_xticks)
    plot_stats_category(filename, category='mana', intervall=intervall, n_xticks=n_xticks)
    plot_stats_category(filename, category='soldiers', intervall=intervall, n_xticks=n_xticks)
    plot_stats_category(filename, category='gold', intervall=intervall, n_xticks=n_xticks)

    # Testrecording, random, 12 players

    # stack_chart()
