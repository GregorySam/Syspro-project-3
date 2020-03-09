/*
 * List.h
 *
 *  Created on: Mar 6, 2018
 *      Author: greg
 */

#ifndef LIST_H_
#define LIST_H_
#include <stdlib.h>



class Node
{
	Node* next;
	char* data;
public:

	char*  Get_Data_Cpy()
	{
		char* ret;
		asprintf(&ret,"%s",data);
		return ret;
	}

	Node* Get_Next()
	{
		return next;
	}
	void Set_NextNULL()
	{
		next=NULL;
	}
	void  Set_Next(char* str)
	{

		next=new Node(str);
	}

	Node(char* Data):next(NULL)
	{
		asprintf(&data,"%s",Data);
	}
	~Node(){free(data);}
};

class List {
	Node *Head;
	Node *End;
	unsigned int size;
public:

	List()
	{
		size=0;
		Head=NULL;
		End=NULL;
	}
	char* Remove_HeadData()
	{
		Node* temp=Head;
		char* data;

		data=Head->Get_Data_Cpy();
		temp = temp->Get_Next();
		
		if(Head==End)
		{
			delete Head;
			Head=temp;
			End=temp;
		}
		else
		{
			delete Head;
			Head=temp;
		}
    size--;

		return data;
	}
	void Insert(char* new_str)
	{
		Node* current;
		size++;

		current=End;

		if(current==NULL)
		{
			current=new Node(new_str);
			End=current;
			Head=current;
			return;
		}
		else
		{
			current->Set_Next(new_str);
			End=current->Get_Next();
		}

	}
	bool Is_Empty()
	{
		return size==0;
	}

	~List()
	{
		Node* current=Head,*next;

		while(current!=NULL)
		{
			next=current->Get_Next();

			delete current;
			current=NULL;
			current=next;
		}
		Head=NULL;
		End=NULL;

	}
};

#endif /* LIST_H_ */
