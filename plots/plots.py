import ijson
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import time

# Apply custom style sheet globally
plt.style.use('custom-dark')


# Get json data of player i by property ['name', 'team', 'race', 'points', 'settlers', 'buildings', 'food', 'mines', 'gold', 'mana', 'soldiers', 'fights']
def get_player_values(filename, player_number, property):
    with open(filename, 'rb') as f:
        object = ijson.items(f, 'stats.player' + str(player_number) + '.' + property)
        for data in object:
            return data


# Get json about game by property ['date', 'time', 'numberOfPlayers', 'map', 'rules']
def get_general_values(filename, property):
    with open(filename, 'rb') as f:
        object = ijson.items(f, 'general.' + property)
        for data in object:
            return data


# Append multiple saves 
def combine_values(filename_list, player_number, property):
    combined_values = np.array([], dtype=np.int64)
    for filename in filename_list:
        values = np.array(get_player_values(filename, player_number, property), dtype=np.int64)
        combined_values = np.append(combined_values, values, axis=0)

    return combined_values


# Stack all players to 2d-array
def stack_values(filename_list, property):
    stacked_values = combine_values(filename_list, 0, property)
    for i in range(1, 4):  # TODO: change to taken spots -> player name required
        combined_values = combine_values(filename_list, i, property)
        if np.count_nonzero(combined_values) > 0:  # TODO: change to taken spots -> player name required
            stacked_values = np.vstack((stacked_values, combined_values))

    # Clean-up
    while(1):
        cleanup_reference = np.sum(stacked_values, axis=0)
        zero_indices = np.where(cleanup_reference == 0)
        startpoint = np.amax(zero_indices) + 1  # startpoint of the last save
        startpoint_value = cleanup_reference[startpoint]
        possible_savepoints = np.where(cleanup_reference[:startpoint] >= startpoint_value)[0]  # find approximate save time
        if possible_savepoints.size == 0:
            break
        savepoint = possible_savepoints[0]
        stacked_values = np.delete(stacked_values, np.arange(savepoint, startpoint), axis=1)

    return stacked_values


def plot_stats_category(filename_list, category, intervall=None, n_xticks=None):
    fig, ax = plt.subplots() 

    # Plot curves
    data = stack_values(filename_list, category)
    for i in range(data.shape[0]):
        y = data[i][:]
        print(i, 'recorded values:', y.size, ', stat endvalue:', y[-1])
        if intervall:  # TODO: automatically set interval depending on game tick
            ax.plot(y[intervall[0]:intervall[1]], label='Player'+str(i))  # TODO: player name 
        else:
            ax.plot(y, label='Player'+str(i)) 

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
    # plot_stats_category(filename, category='buildings', intervall=intervall, n_xticks=n_xticks)
    # plot_stats_category(filename, category='mana', intervall=intervall, n_xticks=n_xticks)
    # plot_stats_category(filename, category='soldiers', intervall=intervall, n_xticks=n_xticks)
    # plot_stats_category(filename, category='gold', intervall=intervall, n_xticks=n_xticks)

    # Testrecording, random, 12 players
    filename_list = ['example/s3-2021-03-22-21-12-33.json','example/s3-2021-03-22-22-25-51.json',
                     'example/s3-2021-03-22-22-30-11.json','example/s3-2021-03-22-22-56-38.json']
    plot_stats_category(filename_list, category='points')

    # stack_chart()
