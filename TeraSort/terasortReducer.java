
import java.io.IOException;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.Counter;
import org.apache.hadoop.util.StringUtils;


public class terasortReducer extends Reducer<Text, Text, Text, Text> 
{
	@Override
	public void reduce(Text key, Iterable<Text> value, Context con) throws IOException, InterruptedException 
	{
		
		String new_val = new String();

		for (Text k : value) 
		{
			
			new_val = k.toString();
			
		}
		
		con.write(key, new Text(new_val));
	}
}

