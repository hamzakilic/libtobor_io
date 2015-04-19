/*
 ============================================================================
 Name        : exampleProgram.c
 Author      : Hamza Kılıç
 Version     :
 Copyright   : Free To Use
 Description : Uses shared library to print greeting
               To run the resulting executable the LD_LIBRARY_PATH must be
               set to ${project_loc}/libtoborio/.libs
               Alternatively, libtool creates a wrapper shell script in the
               build directory of this program which can be used to run it.
               Here the script will be called exampleProgram.
 ============================================================================
 */


#include "em_io.h"


int test_all_pins(){
	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
	  if(em_io_initialize(0))
		  return 1;
	int i=0;
	for(i=0;i<17;++i)
	{
		 if(em_io_gpio_mode(i,EM_DIRECTION_OUT))
			  return 1;
		 if(em_io_gpio_write(i,EM_GPIO_LOW))
		 		   return 1;
	}


	i=0;
	int total_spin=0;
	//for(total_spin=0;total_spin<2;++total_spin)
	for(i=0;i<17;++i){

		if(em_io_gpio_write(i,EM_GPIO_HIGH))
		   return 1;
	   sleep(1);
	   if(em_io_gpio_write(i,EM_GPIO_LOW))
	   	   return 1;


	  }


}
int test_read_pins(){
	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
		  if(em_io_initialize(0))
			  return 1;
		int i=0;
		for(i=0;i<17;++i)
		{
			 if(em_io_gpio_mode(i,EM_DIRECTION_IN))
				  return 1;


		}
		em_io_gpio_mode(1,EM_DIRECTION_OUT);
		em_io_gpio_write(1,EM_GPIO_HIGH);
		em_uint8 read_val;
		for(i=0;i<100000;++i)
		{
			em_io_gpio_read(EM_GPIO_5,&read_val);
			if(read_val & EM_GPIO_LOW)
			fprintf(stdout,"read value is:low\n");
			else
				if(read_val & EM_GPIO_HIGH)
							fprintf(stdout,"read value is:high\n");
				else fprintf(stdout,"unknown value is readed:%d\n",read_val);

		}
		return 1;
}
int test_pull_up_down(){

	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
			  if(em_io_initialize(0))
				  return 1;
			int i=0;

			if(em_io_gpio_mode(EM_GPIO_5,EM_DIRECTION_IN))
					  return 1;


			em_uint8 read_val;
			em_io_gpio_pull(EM_GPIO_5, EM_PULL_UP);
			sleep(1);
			for(i=0;i<100000;++i)
			{
				em_io_gpio_read(EM_GPIO_5,&read_val);
				if(read_val & EM_GPIO_LOW)
				fprintf(stdout,"read value 5 is:low\n");
				else
					if(read_val & EM_GPIO_HIGH)
								fprintf(stdout,"read value 5 is:high\n");
					else fprintf(stdout,"unknown value is readed:%d\n",read_val);



			}
			return 1;

}

em_uint32 test_all_up_read(){

	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
				  if(em_io_initialize(0))
					  return 1;
				int i=0;
             for(i=0;i<17;++i)
				if(em_io_gpio_mode(i,EM_DIRECTION_IN))
						  return 1;
             em_uint8 val;
             for(i=0;i<17;++i)
             {
            	 em_io_gpio_read(i,&val);
            	 if(val& EM_GPIO_LOW)
            		 fprintf(stdout,"%d is low\n",i);
            	 else
            		 fprintf(stdout,"%d is high\n",i);
             }
             fprintf(stdout,"all up ********************press a key\n");
             getchar();

             for(i=0;i<17;++i)
            	 em_io_gpio_pull(i,EM_PULL_UP);
             for(i=0;i<17;++i)
                         {
                        	 em_io_gpio_read(i,&val);
                        	 if(val& EM_GPIO_LOW)
                        		 fprintf(stdout,"%d is low\n",i);
                        	 else
                        		 fprintf(stdout,"%d is high\n",i);
                         }


             fprintf(stdout,"all down ********************press a key\n");
             getchar();
                          for(i=0;i<17;++i)
                         	 em_io_gpio_pull(i,EM_PULL_DOWN);
                          for(i=0;i<17;++i)
                                      {
                                     	 em_io_gpio_read(i,&val);
                                     	 if(val& EM_GPIO_LOW)
                                     		 fprintf(stdout,"%d is low\n",i);
                                     	 else
                                     		 fprintf(stdout,"%d is high\n",i);
                                      }

}

em_uint32 test_events(){


	em_log(EM_LOG_INFO,0,"hadi bakalım t est events\n");
			  if(em_io_initialize(0))
				  return 1;
			  int i=0;
			  em_uint8 val=0;
			  for(i=0;i<17;++i){
			    em_io_gpio_read_event(i,&val);
			    if(val & EM_EVENT_DETECTED)
			    	fprintf(stdout,"%d pin event detected\n",i);
			    else
			    	fprintf(stdout,"%d pin event not detected\n",i);
			  }




			for(i=0;i<17;++i)
			{
				 if(em_io_gpio_mode(i,EM_DIRECTION_IN))
					  return 1;
				 em_io_gpio_pull(i,EM_PULL_DOWN);


			}
			em_io_gpio_mode(EM_GPIO_0,EM_DIRECTION_OUT);
			em_io_gpio_write(EM_GPIO_0,EM_GPIO_HIGH);
			fprintf(stdout,"executing set event\n");
			for(i=0;i<17;++i)
			em_io_gpio_set_event(i,EM_EVENT_RISING_EDGE_DETECT | EM_EVENT_FALLING_EDGE_DETECT | EM_EVENT_HIGH_DETECT | EM_EVENT_LOW_DETECT, EM_EVENT_DISABLE);
			em_log(EM_LOG_INFO,0,"executing set event 2\n");
			//for(i=0;i<17;++i)
			//em_io_gpio_set_event(i, EM_EVENT_HIGH_DETECT|EM_EVENT_LOW_DETECT|EM_EVENT_EDGE_DETECT ,EM_EVENT_ENABLE);
           // getchar();
			em_io_gpio_set_event(EM_GPIO_16, EM_EVENT_HIGH_DETECT  ,EM_EVENT_ENABLE);
			em_uint8 read_val;
			for(i=0;i<100000;++i)
			{

				/*em_io_gpio_read(EM_GPIO_16,&read_val);
				if(read_val & EM_GPIO_LOW)
				fprintf(stdout,"read value is:low\n");
				else
					if(read_val & EM_GPIO_HIGH)
								fprintf(stdout,"read value is:high\n");
					else fprintf(stdout,"unknown value is readed:%d\n",read_val);*/

                em_io_gpio_read_event(EM_GPIO_16,&val);
                if(val & EM_EVENT_DETECTED)
               			    	fprintf(stdout,"%d pin event detected %d \n",EM_GPIO_16,sizeof(em_uint32));
                else 		    	fprintf(stdout,"no event \n");

                em_io_delay_microseconds(500);

			}
			return 1;
}

int test_time(){
	  if(em_io_initialize(0))
					  return 1;
	  em_uint64 time=0;
	  int counter=0;
	  for(counter=0;counter <10 ;++counter){
		  em_io_current_time(&time);
		  fprintf(stdout,"time is:%lld microsecond\n ",time);
		  em_io_delay_microseconds(1*100000);
	  }
}



int main(void) {


test_time();


  return 0;
}
