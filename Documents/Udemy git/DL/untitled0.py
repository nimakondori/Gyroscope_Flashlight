# -*- coding: utf-8 -*-
"""
Created on Wed Apr 17 18:18:49 2019

@author: n_kon
"""

#Install Theano

#Install Tensorflow

#Install Keras library

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd


#Importing the Dataset
dataset = pd.read_csv('Churn_Modelling.csv')
X = dataset.iloc[:, 3: 13].values
Y = dataset.iloc[:, 13].values

#Encode the Categorical Variables
from sklearn.preprocessing import LabelEncoder, OneHotEncoder
labelencoder_X_1 = LabelEncoder()
X[:, 1] = labelencoder_X_1.fit_transform(X[:, 1])
labelencoder_X_2 = LabelEncoder()
X[:, 2] = labelencoder_X_2.fit_transform(X[:, 2])
onehotencoder = OneHotEncoder (categorical_features = [1])
X = onehotencoder.fit_transform(X).toarray()
#No more dummy variable trap
X = X[:, 1:]

#SPlitting the dataset into the Training set and Test set
# cross_validation has been replaced with sklearn.model.selction
from sklearn.model_selection import train_test_split 
X_train, X_test, y_train, y_test = train_test_split(X, Y, test_size = 0.2, random_state = 0)

# We still need to scale the features
# Feature Scaling
from sklearn.preprocessing import StandardScaler
sc = StandardScaler ()
X_train = sc.fit_transform(X_train)
X_test = sc.transform(X_test)


#Part 2
#import Keras libraries
import keras
from keras.models import Sequential
from keras.layers import Dense

 # Initializing the  ANN 
classifier = Sequential()
 # Adding the input layer and the first hidden layer 
classifier.add(Dense(output_dim = 6, init = 'uniform', activation = 'relu', input_dim = 11))
# Adding the second hidden layer
# if your dealing with more that 1 category of outputs instead of the sigmoid function 
# use the softmax function
classifier.add(Dense(output_dim = 6, init = 'uniform', activation = 'relu'))



classifier.add(Dense(output_dim = 1, init = 'uniform', activation = 'sigmoid'))

classifier.compile(optimizer = 'adam', loss= 'binary_crossentropy', metrics = ['accuracy']) 


classifier.fit(X_train, y_train, batch_size= 10, nb_epoch = 100)


y_pred = classifier.predict(X_test)

y_pred = (y_pred > 0.5)

from sklearn.metrics import confusion_matrix
cm = confusion_matrix(y_test, y_pred)

##Homework solution

new_prediction = classifier.predict(sc.transform(np.array([[0.0, 0, 600, 1, 40, 3, 60000, 2, 1, 1, 50000]])))

new_prediction = (new_prediction > 0.5)
